#define LIBENCLOUD_DISABLE_TRACE  // disable heave tracing
#include <QRegExp>
#include <QVariantMap>
#include <encloud/Crypto>
#include <encloud/Utils>
#include <common/common.h>
#include <common/config.h>
#include <setup/cc2/cc2msg.h>
#include <encloud/Json>
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

/** 
 * Handle Autoregistration via Switchboard API
 */
CC2Msg::CC2Msg ()
    : MessageInterface()
{
    LIBENCLOUD_RETURN_IF (_init(), );

    clear();
}

CC2Msg::~CC2Msg ()
{
    clear();
}

int CC2Msg::clear ()
{
    LIBENCLOUD_TRACE;

    MessageInterface::clear();

    _sbAuth = Auth();
    _key = QByteArray();
    _provisioning = QByteArray();
    _regUrl = QUrl();
    _redirectUrl = QUrl();

    return 0;
}

//
// public slots
//

// Try to download a new provisioning file from registry URL - if it doesn't exist use cached file
int CC2Msg::process ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_RETURN_IF (_cfg == NULL, ~0);

    LIBENCLOUD_DBG("[Setup] appdata dir: " << getCommonAppDataDir());
    QByteArray regData;

    loadedConfiguration = false;

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
//    _regUrl.addQueryItem("activationKey", _sbAuth.getPass());
//    _regUrl.addQueryItem("serialNumber", "");
    regData = QString("{ \"activationKey\" : \"%1\",  \"serialNumber\" : \"%2\" }").arg( _sbAuth.getPass()).arg("").toUtf8();

    LIBENCLOUD_DELETE_LATER(_client);
    EMIT_ERROR_ERR_IF ((_client = new Client) == NULL);

    connect(_client, SIGNAL(error(libencloud::Error, QVariant)), this, SLOT(_error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>, QVariant)),
            this, SLOT(_gotConfig(QString, QMap<QByteArray, QByteArray>)));

    LIBENCLOUD_NOTICE("[Setup] Requesting config from URL: " << _regUrl.host());

    _client->setVerifyCA(_cfg->config.sslInit.verifyCA);
    // Use post version to avoid logging sensitive informations
    _headers["Content-Type"] = "application/json";
    _client->post(_regUrl, _headers, regData, _sslconf);

    return 0;
err:
    LIBENCLOUD_DELETE_LATER(_client);
    return ~0;
}

void CC2Msg::authSupplied (const libencloud::Auth &auth)
{
    LIBENCLOUD_DBG("url: " << auth.getUrl() << ", user: " << auth.getUser());

    if (auth.getType() != Auth::ActivationCodeType) {
        // ignore his own supplied credentials via authChanged
        return;
    }
    
    switch (auth.getId())
    {
        case Auth::SwitchboardId:
            if (auth.getId() == _sbAuth.getId() && auth.getPass() == _sbAuth.getPass()) {
                LIBENCLOUD_DBG("Reusing existing credentials for same activation code. A full stop/start is required to get VPN credentials");
                // need to change to avoid overwriting actual auth with activation code
                emit authChanged(_retrievedSbAuth);
            } else {
                LIBENCLOUD_DBG("Activation code has changed. A full stop/start is required to get the new VPN credentials");
                _sbAuth = auth;
                _retrievedSbAuth = Auth();
            }
            break;
        default:
            // Qt Proxy is handled globally in core
            break;
    }
}

//
// private slots
// 

void CC2Msg::_error (const libencloud::Error &err)
{
    LIBENCLOUD_RETURN_IF (_cfg == NULL, );

    LIBENCLOUD_EMIT_RETURN_IF (err.getCode() != libencloud::Error::CodeServerNotFound, error(err), );
    LIBENCLOUD_EMIT_RETURN_IF (!loadedConfiguration, error(Error(Error::CodeUnregActivation)), );

err:
    emit error(Error(Error::CodeSetupFailure));
}

// Step 2: download config
void CC2Msg::_gotConfig (const QString &response, const QMap<QByteArray, QByteArray> &headers)
{
    LIBENCLOUD_UNUSED(headers);

    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_decodeConfig(response));
    loadedConfiguration = true;

    // detached slot since _client is reused
    QTimer::singleShot (0, this, SLOT(_completeSetup()));

    return;

err:
    emit error(Error(Error::CodeSetupFailure));
}

int CC2Msg::_decodeConfig (const QString &response)
{
    bool ok = false;
    QVariantMap object = json::parse(response, ok).toMap();
    LIBENCLOUD_ERR_MSG_IF(!ok, "Failed parsing provisioning content");
    LIBENCLOUD_ERR_MSG_IF (object.value("vpnKey").toString().isEmpty(), "Missing required field vpnField");
    LIBENCLOUD_ERR_MSG_IF (object.value("vpnPairingApiUrl").toString().isEmpty(), "Missing required field vpnPairingApiUrl");
    
    {
        _retrievedSbAuth = _sbAuth;
        QUrl sbUrl(object.value("vpnPairingApiUrl").toString());
        sbUrl.setPath("");
        if (sbUrl.toString().isEmpty()) {
            LIBENCLOUD_NOTICE("Using default sb url " << _cfg->config.sbUrl.toString() );
            _retrievedSbAuth.setUrl(  _cfg->config.sbUrl.toString() );
        } else {
            _retrievedSbAuth.setUrl(sbUrl.toString());
        }

        QByteArray userPass = QByteArray::fromBase64( object.value("vpnKey").toString().toUtf8() );
        QList<QByteArray> userAndPass = userPass.split(':');
        LIBENCLOUD_ERR_MSG_IF( userAndPass.length() != 2, "Failed decoding vpnKey field" );
        _retrievedSbAuth.setUser(userAndPass[0]);
        _retrievedSbAuth.setPass(userAndPass[1]);
        _retrievedSbAuth.setType(Auth::UserpassType);
        emit authChanged(_retrievedSbAuth);
    }
    
    return 0;

err:
    return ~0;
}

//// Complete setup operation by deleting registry resources
void CC2Msg::_completeSetup ()
{
    LIBENCLOUD_DELETE_LATER(_client);
    LIBENCLOUD_ERR_IF ((_client = new Client) == NULL);

err:
    // non-critical failures - setup complete
    emit processed();
}

int CC2Msg::_init ()
{
    LIBENCLOUD_ERR_IF (libencloud_crypto_init(&_ec));
    LIBENCLOUD_ERR_IF (libencloud_crypto_set_cipher(&_ec, crypto::Aes256Cfb8Cipher));
    LIBENCLOUD_ERR_IF (libencloud_crypto_set_digest(&_ec, crypto::Sha256Digest));

    return 0;
err:
    return ~0;
}

QByteArray CC2Msg::_getCode ()
{
    return _sbAuth.getPass().toLatin1();
}

// Registry path is based on hash(hash(AC))
QByteArray CC2Msg::_calcRegPath ()
{
    QByteArray h = crypto::hash(&_ec, _calcConfigPath());

    LIBENCLOUD_ERR_IF (h.isEmpty());

    return utils::base642Url(h.toBase64());
err:
    return QByteArray();

}

// Config path is based on hash(AC)
QByteArray CC2Msg::_calcConfigPath ()
{
    QByteArray h = crypto::hash(&_ec, _getCode());

    LIBENCLOUD_ERR_IF (h.isEmpty());

    return utils::base642Url(h.toBase64());
err:
    return QByteArray();
}


} // namespace libencloud
