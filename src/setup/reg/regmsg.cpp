#define LIBENCLOUD_DISABLE_TRACE  // disable heave tracing
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

namespace libencloud {

//
// public methods
//

/*
Handle Autoregistration via Switchboard API

# Shell-based proof of concept
HOST="connect.endian.com"
ACTIVATION_CODE="XXXX-XXXX-XXXX"
KEY=$(echo -n $ACTIVATION_CODE | openssl dgst -sha256 -binary)
KEY_HEX=$(echo -n $KEY | od -A n -v -t x1 | tr -d ' \n')
URL=https://$HOST/manage/access/registry/$(echo -n $KEY | openssl base64 | sed 's/\//_/g' | sed 's/=$//g')
curl -A 'Endian 4i Connect' -k $URL | sed 's/-----\(.*\)ENCRYPTED PROVISONING -----//g' | \
    base64 --decode | openssl enc -d -aes-256-cfb8 -K $KEY_HEX -iv 0 | tail -c +49 # 16(salt)+32(csum)+1
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
    _code = QString();
    _key = QByteArray();
    _provisioningEnc = QByteArray();

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

    QUrl url;
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration sslconf;
    
    LIBENCLOUD_RETURN_IF (_cfg == NULL, ~0);

    LIBENCLOUD_DBG("[Setup] appdata dir: " << getCommonAppDataDir());

    if (_sbAuth.getUrl().isEmpty())
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

    // Switchboard is strict on this
    headers["User-Agent"] = LIBENCLOUD_USERAGENT_QCC;

    LIBENCLOUD_DBG("[Setup] User Agent: " << headers["User-Agent"]);

    url.setUrl(_sbAuth.getUrl());
    url.setPath(QString(LIBENCLOUD_SETUP_QCC_REG_URL) + '/' + _calcRegPath());

    LIBENCLOUD_NOTICE("[Setup] Requesting configuration from URL: " << url.toString());

    LIBENCLOUD_DELETE_LATER(_client);
    EMIT_ERROR_ERR_IF ((_client = new Client) == NULL);

    // setup signals from client
    connect(_client, SIGNAL(error(libencloud::Error)), this, SLOT(_error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>)),
            this, SLOT(_clientComplete(QString, QMap<QByteArray, QByteArray>)));

    _client->setVerifyCA(_cfg->config.sslInit.verifyCA);
    _client->run(url, params, headers, sslconf);

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

    if (err.getCode() != libencloud::Error::CodeServerNotFound ||
            !pf.exists())
    {
        emit error(err);
        return;
    }

    LIBENCLOUD_DBG("Reading cached provisioning file");

    QByteArray config;
    LIBENCLOUD_ERR_IF (!pf.open(QIODevice::ReadOnly));

    config = _decrypt(pf.readAll());
    pf.close();
    LIBENCLOUD_ERR_IF (config.isEmpty());
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

void RegMsg::_clientComplete (const QString &response, const QMap<QByteArray, QByteArray> &headers)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_decodeResponse(response, headers));
    LIBENCLOUD_ERR_IF (_unpackResponse());

    // detached slot since _client is reused
    QTimer::singleShot (0, this, SLOT(_completeSetup()));
    return;

err:
    emit error(Error(Error::CodeSetupFailure));
}

//
// private methods
// 

int RegMsg::_packRequest ()
{
    return 0;
}

int RegMsg::_encodeRequest (QUrl &url, QUrl &params)
{
    LIBENCLOUD_UNUSED(url);
    LIBENCLOUD_UNUSED(params);

    return 0;
}

int RegMsg::_decodeResponse (const QString &response, const QMap<QByteArray, QByteArray> &headers)
{
    LIBENCLOUD_UNUSED(headers);

    QRegExp headerRx("-----\\s*(?:BEGIN|END) ENCRYPTED PROVISONING\\s*-----");

    QString s(response);
    s.remove(headerRx);

    QByteArray enc = QByteArray::fromBase64(s.toAscii());
    QByteArray config;

    config = _decrypt(enc);
    LIBENCLOUD_ERR_IF (config.isEmpty());

    LIBENCLOUD_ERR_IF (_decodeConfig(config));
    _provisioningEnc = enc;

    return 0;
err:
    return ~0;
}

int RegMsg::_unpackResponse ()
{
    LIBENCLOUD_RETURN_IF (_cfg == NULL, ~0);

    QFile pf(_cfg->config.regProvisioningPath.absoluteFilePath());
    QFile caf(_cfg->config.sslOp.caPath.absoluteFilePath());

    // save encrypted provisioning file
    LIBENCLOUD_ERR_IF (!utils::fileCreate(pf, QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (pf.write(_provisioningEnc) == -1);
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

// Complete setup operation by deleting registry resource
void RegMsg::_completeSetup ()
{
    QUrl url;
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration sslconf;

    // Switchboard is strict on this
    headers["User-Agent"] = LIBENCLOUD_USERAGENT_QCC;

    url.setUrl(_sbAuth.getUrl());
    url.setPath(QString(LIBENCLOUD_SETUP_QCC_REG_URL) + '/' + _calcRegPath());

    LIBENCLOUD_DELETE_LATER(_client);
    EMIT_ERROR_ERR_IF ((_client = new Client) == NULL);

    _client->setVerifyCA(false);
    _client->del(url, headers, sslconf);

    emit processed();
    return;
err:
    LIBENCLOUD_DELETE_LATER(_client);
    emit error(Error(Error::CodeSetupFailure));
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

QString RegMsg::_getCode ()
{
    if (!_code.isNull())
        return _code;

    _code = getActivationCode(false).toUpper();  // unencrypted

    return _code;
}

// Calculate key = sha256(activation_code)
QByteArray RegMsg::_getKey ()
{
    if (!_key.isNull())
        return _key;

    unsigned char md[LIBENCLOUD_CRYPTO_MAX_MD_SZ];
    unsigned int md_sz;
    QString code = _getCode();

    LIBENCLOUD_ERR_IF (libencloud_crypto_digest(&ec, (unsigned char *) code.toAscii().data(), code.toAscii().size(),
        md, &md_sz));

    return (_key = QByteArray((const char *)md, md_sz));
err:
    return QByteArray();
}

QString RegMsg::_calcRegPath ()
{
    QByteArray key = _getKey();

    LIBENCLOUD_ERR_IF (key.isEmpty());

    return utils::base642Url(key.toBase64());
err:
    return QString();
}

QByteArray RegMsg::_decrypt (const QByteArray &enc)
{
    enum {
        SALT_SZ = 16,
        CSUM_SZ = 32,
        HDR_SZ = SALT_SZ + CSUM_SZ,
        PTEXT_SZ = 4096
    };
    QByteArray key = _getKey();
    unsigned char iv[SALT_SZ];
    unsigned char ptext[PTEXT_SZ];
    long ptext_sz;

    memset(iv, 0, sizeof(iv));

    LIBENCLOUD_ERR_IF (libencloud_crypto_dec (&ec, (unsigned char *) enc.data(), enc.size(), 
                (unsigned char *) key.data(), iv, ptext, &ptext_sz));
    LIBENCLOUD_ERR_IF (ptext_sz < HDR_SZ);

    // verify checksum
    LIBENCLOUD_ERR_IF (key != QByteArray((const char *)&ptext[SALT_SZ], CSUM_SZ));

    return QByteArray((const char *)&ptext[HDR_SZ], (ptext_sz - HDR_SZ));
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

    // Note::YAML API functions may throw exceptions (against Qt standard)! 
    // caught by Encloud::Application::notify()
    std::stringstream stream(std::string(config.constData(), config.length()));
    YAML::Parser parser(stream);

    LIBENCLOUD_ERR_IF (!parser.GetNextDocument(node));
    LIBENCLOUD_ERR_IF (!node.size());

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
