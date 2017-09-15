#define LIBENCLOUD_DISABLE_TRACE  // disable heave tracing
#include <QRegExp>
#include <QVariantMap>
#include <encloud/Utils>
#include <common/common.h>
#include <common/config.h>
#include <setup/reg/regmsg.h>
#if defined(LIBENCLOUD_MODE_QCC) && !defined(LIBENCLOUD_SPLITDEPS)
#  include <common/qcc_version.h>
#endif

// use only to wrap upper-level methods, otherwise duplicates will be emitted
#define EMIT_ERROR_ERR_IF(cond) \
    LIBENCLOUD_EMIT_ERR_IF(cond, error(Error(Error::CodeGeneric)))

#define LIBENCLOUD_SETUP_REGMSG_HDR    "-----\\s*(?:BEGIN|END) ENCRYPTED PROVISONING\\s*-----"

namespace libencloud {

//
// public methods
//

/** 
 * Handle Autoregistration via Switchboard API
 */
RegMsg::RegMsg ()
    : MessageInterface()
{
    LIBENCLOUD_RETURN_IF (_init(), );

    clear();
}

int RegMsg::clear ()
{
    LIBENCLOUD_TRACE;

    MessageInterface::clear();

    _sbAuth = Auth();
    _vpnConfig.clear();
    _fallbackVpnConfig.clear();
    _caCert.clear();
    _key = QByteArray();
    _provisioning = QByteArray();
    _regUrl = QUrl();
    _redirectUrl = QUrl();

    return 0;
}

const VpnConfig *RegMsg::getVpnConfig () const
{
    return &_vpnConfig;
}

const VpnConfig *RegMsg::getFallbackVpnConfig () const
{
    return &_fallbackVpnConfig;
}

//
// public slots
//

// Try to download a new provisioning file from registry URL - if it doesn't exist use cached file
int RegMsg::process ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_RETURN_IF (_cfg == NULL, ~0);

    LIBENCLOUD_DBG("[Setup] appdata dir: " << getCommonAppDataDir());

    if (_sbAuth.getUrl().isEmpty() || _getCode().isEmpty())
    {
        LIBENCLOUD_DBG("No login info - doing nothing");
        emit error(Error(Error::CodeUnconfigured));
        return 0;
    }

    if (!_sbAuth.isValid())
    {
        emit authRequired(Auth::SwitchboardId, QVariant());
        LIBENCLOUD_EMIT_ERR (error(Error(tr("Switchboard login required"))));
    }

    _regUrl.setUrl(_sbAuth.getUrl());
    _regUrl.setPath(QString(LIBENCLOUD_SETUP_QCC_REG_URL) + '/' + _calcRegPath());

    LIBENCLOUD_DELETE_LATER(_client);
    EMIT_ERROR_ERR_IF ((_client = new Client) == NULL);

    connect(_client, SIGNAL(error(libencloud::Error, QVariant)), this, SLOT(_error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>, QVariant)),
            this, SLOT(_gotRedirect(QString, QMap<QByteArray, QByteArray>)));

    LIBENCLOUD_NOTICE("[Setup] Requesting redirect from URL: " << _regUrl.toString());

    _client->setVerifyCA(_cfg->config.sslInit.verifyCA);
    _client->run(_regUrl, _params, _headers, _sslconf);

    return 0;
err:
    LIBENCLOUD_DELETE_LATER(_client);
    return ~0;
}

void RegMsg::authSupplied (const libencloud::Auth &auth)
{
    LIBENCLOUD_DBG("url: " << auth.getUrl() << ", user: " << auth.getUser());

    switch (auth.getId())
    {
        case Auth::SwitchboardId:
            _sbAuth = auth;
            break;
        default:
            // Qt Proxy is handled globally in core
            break;
    }
}

//
// private slots
// 

void RegMsg::_error (const libencloud::Error &err)
{
    LIBENCLOUD_RETURN_IF (_cfg == NULL, );

    QFile caf(_cfg->config.sslOp.caPath.absoluteFilePath());
    QFile pf(_cfg->config.regProvisioningPath.absoluteFilePath());

    LIBENCLOUD_DBG("err: " << err.toString());

    LIBENCLOUD_EMIT_RETURN_IF (err.getCode() != libencloud::Error::CodeServerNotFound, error(err), );
    LIBENCLOUD_EMIT_RETURN_IF (!pf.exists(), error(Error(Error::CodeUnregActivation)), );

    LIBENCLOUD_DBG("Reading cached provisioning file");

    QByteArray config;
    LIBENCLOUD_ERR_IF (!pf.open(QIODevice::ReadOnly));

    config = _decrypt(_hash(_getCode()), pf.readAll());
    pf.close();
    LIBENCLOUD_EMIT_RETURN_IF (config.isEmpty(), error(Error(Error::CodeBadActivation)), );  
    LIBENCLOUD_ERR_IF (_decodeConfig(config));

    // save the Operation CA certificate to file
    LIBENCLOUD_ERR_IF (!utils::fileCreate(caf, QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (caf.write(_caCert.toPem()) == -1);
    caf.close();

    emit processed();  // success
    return;

err:
    emit error(Error(Error::CodeSetupFailure));
}

// Step 1: get redirect URL
void RegMsg::_gotRedirect (const QString &response, const QMap<QByteArray, QByteArray> &headers)
{
    LIBENCLOUD_UNUSED(headers);

    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_decodeRedirect(response));

    // detached slot since _client is reused
    QTimer::singleShot (0, this, SLOT(_processRedirect()));

    return;
err:
    emit error(Error(Error::CodeSetupFailure));
}

// Step 2: download config
void RegMsg::_gotConfig (const QString &response, const QMap<QByteArray, QByteArray> &headers)
{
    LIBENCLOUD_UNUSED(headers);

    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_decodeConfig(response));
    LIBENCLOUD_ERR_IF (_unpackConfig());

    // detached slot since _client is reused
    QTimer::singleShot (0, this, SLOT(_completeSetup()));

    return;

err:
    emit error(Error(Error::CodeSetupFailure));
}

//
// private methods
// 

int RegMsg::_decodeRedirect (const QString &response)
{
    QRegExp headerRx(LIBENCLOUD_SETUP_REGMSG_HDR);
    QRegExp redirRx("REDIRECT: (.*)");

    QString s(response);
    s.remove(headerRx);

    QByteArray enc = QByteArray::fromBase64(s.toAscii());
    QByteArray redirect;

    redirect = _decrypt(_hash(_getCode()), enc);
    LIBENCLOUD_ERR_IF (redirect.isEmpty());

    LIBENCLOUD_ERR_IF (redirRx.indexIn(redirect) == -1);
    _redirectUrl = QUrl(redirRx.cap(1).trimmed());
    LIBENCLOUD_ERR_IF (!_redirectUrl.isValid());

    return 0;
err:
    return ~0;
}

void RegMsg::_processRedirect ()
{
    LIBENCLOUD_DELETE_LATER(_client);
    EMIT_ERROR_ERR_IF ((_client = new Client) == NULL);

    connect(_client, SIGNAL(error(libencloud::Error, QVariant)), this, SLOT(_error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>, QVariant)),
            this, SLOT(_gotConfig(QString, QMap<QByteArray, QByteArray>)));

    LIBENCLOUD_NOTICE("[Setup] Requesting configuration from URL: " << _redirectUrl.toString());

    // registry Switchboard has been verified - so we also trust redirects
    _client->setVerifyCA(false);
    _client->run(_redirectUrl, _params, _headers, _sslconf);

    return;
err:
    LIBENCLOUD_DELETE_LATER(_client);
    return;
}

int RegMsg::_decodeConfig (const QString &response)
{
    QRegExp headerRx(LIBENCLOUD_SETUP_REGMSG_HDR);

    QString s(response);
    s.remove(headerRx);
    QUrl sbUrl(_redirectUrl);

    QByteArray enc = QByteArray::fromBase64(s.toAscii());

    _provisioning = _decrypt(_hash(_getCode()), enc);
    LIBENCLOUD_ERR_IF (_provisioning.isEmpty());

    //
    // append local bits
    //
    sbUrl.setPath("");
    _provisioning += "\nQCC_SWITCHBOARD_URL: " + sbUrl.toString();

    LIBENCLOUD_ERR_IF (_decodeConfig(_provisioning));

//#define LIBENCLOUD_SETUP_REG_DEBUG
#ifdef LIBENCLOUD_SETUP_REG_DEBUG
{
    // save UN-encrypted provisioning file
    QFile pf(_cfg->config.regProvisioningPath.absoluteFilePath() + ".dbg");
    LIBENCLOUD_ERR_IF (!utils::fileCreate(pf, QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (pf.write(_provisioning) == -1);
    pf.close();
}
#endif

    return 0;
err:
    return ~0;
}

int RegMsg::_unpackConfig ()
{
    QByteArray provisioningEnc;

    LIBENCLOUD_RETURN_IF (_cfg == NULL, ~0);

    QFile pf(_cfg->config.regProvisioningPath.absoluteFilePath());
    QFile caf(_cfg->config.sslOp.caPath.absoluteFilePath());

    // save encrypted provisioning file
    LIBENCLOUD_ERR_IF (!utils::fileCreate(pf, QIODevice::WriteOnly));
    provisioningEnc = _encrypt(_hash(_getCode()), _provisioning);
    LIBENCLOUD_ERR_IF (pf.write(provisioningEnc) == -1);
    pf.close();

    // save the Operation CA certificate to file
    LIBENCLOUD_ERR_IF (!utils::fileCreate(caf, QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (caf.write(_caCert.toPem()) == -1);
    caf.close();

    return 0;
err:
    emit (error(Error(Error::CodeSystemError, tr("Failed writing Operation CA: ") +
                    caf.errorString())));

    pf.close();
    caf.close();
    return ~0;
}

// Complete setup operation by deleting registry resources
void RegMsg::_completeSetup ()
{
    LIBENCLOUD_DELETE_LATER(_client);
    LIBENCLOUD_ERR_IF ((_client = new Client) == NULL);

    LIBENCLOUD_NOTICE("[Setup] Requesting deletion of URLs: " << 
            _redirectUrl.toString() << ", " << _regUrl.toString());

    // registry Switchboard has been verified - so we also trust redirects
    _client->setVerifyCA(false);
    _client->del(_redirectUrl, _headers, _sslconf);
    _client->del(_regUrl, _headers, _sslconf);

err:
    // non-critical failures - setup complete
    emit processed();
}

int RegMsg::_init ()
{
    LIBENCLOUD_ERR_IF (libencloud_crypto_init(&ec));
    LIBENCLOUD_ERR_IF (libencloud_crypto_set_cipher(&ec, libencloud::crypto::Aes256Cfb8Cipher));
    LIBENCLOUD_ERR_IF (libencloud_crypto_set_digest(&ec, libencloud::crypto::Sha256Digest));

    return 0;
err:
    return ~0;
}

QByteArray RegMsg::_getCode ()
{
    return getActivationCode(false).toUpper().toAscii();  // unencrypted
}

// Calculate sha256 checksum
QByteArray RegMsg::_hash (const QByteArray &ba)
{
    unsigned char md[LIBENCLOUD_CRYPTO_MAX_MD_SZ];
    unsigned int md_sz;

    LIBENCLOUD_ERR_IF (libencloud_crypto_digest(&ec, (unsigned char *) ba.data(), ba.size(),
        md, &md_sz));

    return QByteArray((const char *)md, md_sz);
err:
    return QByteArray();
}

// Registry path is based on hash(hash(AC))
QByteArray RegMsg::_calcRegPath ()
{
    QByteArray h = _hash(_calcConfigPath());

    LIBENCLOUD_ERR_IF (h.isEmpty());

    return utils::base642Url(h.toBase64());
err:
    return QByteArray();

}

// Config path is based on hash(AC)
QByteArray RegMsg::_calcConfigPath ()
{
    QByteArray h = _hash(_getCode());

    LIBENCLOUD_ERR_IF (h.isEmpty());

    return utils::base642Url(h.toBase64());
err:
    return QByteArray();
}

QByteArray RegMsg::_encrypt (const QByteArray &key, const QByteArray &text)
{
    enum {
        SALT_SZ = 16,
        CSUM_SZ = 32,
        HDR_SZ = SALT_SZ + CSUM_SZ,
        CTEXT_SZ = 8192
    };
    unsigned char hdr[HDR_SZ];
    unsigned char iv[SALT_SZ];
    unsigned char ctext[CTEXT_SZ];
    long ctext_sz;

    memset(iv, 0, sizeof(iv));
    memset(hdr, 0, sizeof(hdr));

    // prepend key as checksum 
    memcpy(&hdr[SALT_SZ], key.data(), CSUM_SZ);

    QByteArray ptext((const char *) hdr, sizeof(hdr));
    ptext += text;

    LIBENCLOUD_ERR_IF (libencloud_crypto_enc (&ec, (unsigned char *) ptext.data(), ptext.size(), 
                (unsigned char *) key.data(), iv, ctext, &ctext_sz));

    return QByteArray((const char *) ctext, ctext_sz);
err:
    return QByteArray();
}

QByteArray RegMsg::_decrypt (const QByteArray &key, const QByteArray &enc)
{
    enum {
        SALT_SZ = 16,
        CSUM_SZ = 32,
        HDR_SZ = SALT_SZ + CSUM_SZ,
        PTEXT_SZ = 8192
    };
    unsigned char iv[SALT_SZ];
    unsigned char ptext[PTEXT_SZ];
    long ptext_sz;

    memset(iv, 0, sizeof(iv));

    LIBENCLOUD_ERR_IF (libencloud_crypto_dec (&ec, (unsigned char *) enc.data(), enc.size(), 
                (unsigned char *) key.data(), iv, ptext, &ptext_sz));
    LIBENCLOUD_ERR_IF (ptext_sz < HDR_SZ);

    // verify key as checksum
    LIBENCLOUD_ERR_IF (key != QByteArray((const char *)&ptext[SALT_SZ], CSUM_SZ));

    return QByteArray((const char *) &ptext[HDR_SZ], (ptext_sz - HDR_SZ));
err:
    return QByteArray();
}

QString RegMsg::_yamlNodeToStr (const YAML::Node *nodeP)
{
    std::string s;

    *nodeP >> s;

    return QString::fromUtf8(s.c_str());
}

int RegMsg::_yamlNodeToInt (const YAML::Node *nodeP)
{
    int i;

    *nodeP >> i;

    return i;
}

int RegMsg::_decodeConfig (const QByteArray &config)
{
    //LIBENCLOUD_DBG("RegMsg::config: " << config);

    YAML::Node node;
    const YAML::Node *n = NULL;

    // grab actual Switchboard host spec from redirect URL
    QUrl sbUrl(_redirectUrl);

    // Note::YAML API functions may throw exceptions (against Qt standard)! 
    // caught by Encloud::Application::notify()
    std::stringstream stream(std::string(config.constData(), config.length()));
    YAML::Parser parser(stream);

    LIBENCLOUD_ERR_IF (!parser.GetNextDocument(node));
    LIBENCLOUD_ERR_IF (!node.size());

    //
    // parse Switchboard info
    //
    LIBENCLOUD_ERR_IF (_decodeConfigVpn(node));
    LIBENCLOUD_ERR_IF (_decodeConfigFallbackVpn(node));

    LIBENCLOUD_ERR_IF ((n = node.FindValue("VPN_CERTIFICATE")) == NULL);
    _caCert = QSslCertificate(_yamlNodeToStr(n).toLocal8Bit());
    LIBENCLOUD_ERR_IF (!_caCert.isValid());

    LIBENCLOUD_ERR_IF ((n = node.FindValue("ACTIVATION_KEY")) == NULL);
    LIBENCLOUD_ERR_IF (_yamlNodeToStr(n) != _getCode());

    LIBENCLOUD_ERR_IF (_sbAuth.setType(Auth::UserpassType));

    LIBENCLOUD_ERR_IF ((n = node.FindValue("VPN_NAME")) == NULL);
    LIBENCLOUD_ERR_IF (_sbAuth.setUser(_yamlNodeToStr(n)));

    LIBENCLOUD_ERR_IF ((n = node.FindValue("VPN_PASSWORD")) == NULL);
    LIBENCLOUD_ERR_IF (_sbAuth.setPass(_yamlNodeToStr(n)));

    //
    // local additions
    //

    // redirect URL will be undefined if provisioning has been cached =>
    // grab it from config
    if ((n = node.FindValue("QCC_SWITCHBOARD_URL")) != NULL)
        sbUrl = QUrl(_yamlNodeToStr(n));

    sbUrl.setPath("");
    _sbAuth.setUrl(sbUrl.toString());

    emit authChanged(_sbAuth);
    
    /*
        Currently Unused: 
            - PROVISIONING_AUTH_KEY
            - ADMIN_PASSWORD
            - NAME
            - EMAIL
            - COMPANY
            - COUNTRY
            - DOMAIN
            - DOWNLOAD_PATH
            - DOWNLOAD_URL
            - RED_{DEV,IPS,DNS,GATEWAY,TYPE}
            - GREEN_{DEV,IPS}
            - ORANGE_{DEV,IPS}
            - BLUE_{DEV,IPS}
            - HOSTNAME
            - ROOT_PASSWORD
            - SSH_ENABLED
            - SYSTEM_ACCESS_FROM_VPN
            - TIMEZONE
            - VPN_ALLOW_PROVISIONING
            - VPN_DEVICE
    */

    return 0;
err:
    return ~0;
}

int RegMsg::_decodeConfigVpn (const YAML::Node &node)
{
    const YAML::Node *n = NULL;

    LIBENCLOUD_ERR_IF ((n = node.FindValue("VPN_PROTOCOL")) == NULL);
    LIBENCLOUD_ERR_IF (_vpnConfig.setRemoteProto(VpnConfig::protoFromStr(_yamlNodeToStr(n))));

    LIBENCLOUD_ERR_IF ((n = node.FindValue("VPN_TYPE")) == NULL);
    LIBENCLOUD_ERR_IF (_vpnConfig.setDev(VpnConfig::devFromStr(_yamlNodeToStr(n))));

    LIBENCLOUD_ERR_IF ((n = node.FindValue("VPN_IP")) == NULL);
    LIBENCLOUD_ERR_IF (_vpnConfig.setRemote(_yamlNodeToStr(n)));

    LIBENCLOUD_ERR_IF ((n = node.FindValue("VPN_PORT")) == NULL);
    LIBENCLOUD_ERR_IF (_vpnConfig.setRemotePort(_yamlNodeToInt(n)));

    LIBENCLOUD_ERR_IF (!_vpnConfig.checkValid(false));

    return 0;
err:
    return ~0;
}

int RegMsg::_decodeConfigFallbackVpn (const YAML::Node &node)
{
    const YAML::Node *n = NULL;

    if ((n = node.FindValue("FALLBACK_VPN_PROTOCOL")))
        LIBENCLOUD_ERR_IF (_fallbackVpnConfig.setRemoteProto(VpnConfig::protoFromStr(_yamlNodeToStr(n))));

    if ((n = node.FindValue("FALLBACK_VPN_TYPE")))
        LIBENCLOUD_ERR_IF (_fallbackVpnConfig.setDev(VpnConfig::devFromStr(_yamlNodeToStr(n))));

    if ((n = node.FindValue("FALLBACK_VPN_IP")))
        LIBENCLOUD_ERR_IF (_fallbackVpnConfig.setRemote(_yamlNodeToStr(n)));

    if ((n = node.FindValue("FALLBACK_VPN_PORT")))
        LIBENCLOUD_ERR_IF (_fallbackVpnConfig.setRemotePort(_yamlNodeToInt(n)));

    // Fallback configuration is optional - don't check, but still set validity 
    _fallbackVpnConfig.checkValid(false);

    return 0;
err:
    return ~0;
}

} // namespace libencloud
