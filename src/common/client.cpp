#include <encloud/Client>
#include <common/common.h>
#include <common/config.h>

#define EMIT_ERROR(msg) LIBENCLOUD_EMIT(error(Error(msg)))
#define EMIT_ERROR_ERR_IF(cond, msg) LIBENCLOUD_EMIT_ERR_IF(cond, error(Error(msg)))

// Allow debug deactivation via API
#define CLIENT_DBG(msg) do { \
    if (_debug) \
        LIBENCLOUD_DBG(msg); \
    } while (0);

namespace libencloud {

Client::Client ()
    : _reply(NULL)
    , _qnam(NULL)
    , _qnamExternal(false)
    , _verifyCA(true)
    , _debug(true)
    , _sslError(false)
{
    LIBENCLOUD_TRACE;

    _qnam = new QNetworkAccessManager;
    LIBENCLOUD_ERR_IF (_qnam == NULL);

    _connectQnam();

    _timer.setSingleShot(true);
    connect(&_timer, SIGNAL(timeout()), this,
            SLOT(_timeout()));
err:
    return;
}

Client::~Client ()
{
    LIBENCLOUD_TRACE;

    if (!_qnamExternal)
        LIBENCLOUD_DELETE(_qnam);
}

QNetworkAccessManager *Client::getNetworkAccessManager ()
{
    return _qnam;
}

int Client::setNetworkAccessManager (QNetworkAccessManager *qnam)
{
    LIBENCLOUD_ERR_IF (qnam == NULL);
    LIBENCLOUD_ERR_IF (_qnamExternal);  // has already been set externally

    LIBENCLOUD_DELETE (_qnam);
    
    _qnam = qnam;
    _qnamExternal = true;

    _connectQnam();
err:
    return ~0;
}

void Client::setVerifyCA (bool b) 
{
    LIBENCLOUD_DBG("[Client] verify CA: " << b);

    _verifyCA = b;
}

void Client::setDebug (bool b) 
{
    _debug = b;
}

void Client::reset ()
{
    if (_qnamExternal)
        return;

    disconnect(_qnam, NULL, NULL, NULL);

    if (_qnam)
        _qnam->deleteLater();

    _qnam = new QNetworkAccessManager;
    LIBENCLOUD_ERR_IF (_qnam == NULL);

    _connectQnam();
err:
    return;
}

void Client::run (const QUrl &url, const QUrl &params,
		const QMap<QByteArray, QByteArray> &headers, const QSslConfiguration &conf)
{
	_send(MSG_TYPE_NONE, url, headers, params.encodedQuery(), conf);
}

void Client::get (const QUrl &url, const QMap<QByteArray, QByteArray> &headers,
        const QSslConfiguration &conf)
{
	_send(MSG_TYPE_GET, url, headers, "", conf);
}

void Client::post (const QUrl &url, const QMap<QByteArray, QByteArray> &headers,
        const QByteArray &data, const QSslConfiguration &conf)
{
	_send(MSG_TYPE_POST, url, headers, data, conf);
}

void Client::_send (MsgType msgType, const QUrl &url, const QMap<QByteArray, QByteArray> &headers,
		const QByteArray &data, const QSslConfiguration &conf)
{
    CLIENT_DBG("[Client] to: " << url.toString());
    //CLIENT_DBG(" ### >>>>> ### " << data);

    QNetworkRequest request(url);
    _sslError = false;
    _response = "";

    LIBENCLOUD_RETURN_IF (_reply != NULL, );

#ifndef Q_OS_WINCE
    if (conf.caCertificates().count()) {
        LIBENCLOUD_DBG("[Client] CA Cert issuer: " <<
                conf.caCertificates().first().issuerInfo(QSslCertificate::CommonName));
    }
    request.setSslConfiguration(conf);
#endif

    // default headers
    request.setRawHeader("User-Agent", LIBENCLOUD_USERAGENT);
    request.setRawHeader("Host", url.host().toAscii());
    if ((msgType == MSG_TYPE_POST) ||
            (msgType == MSG_TYPE_NONE && !data.isEmpty()))
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // override with passed custom headers
    for (QMap<QByteArray, QByteArray>::const_iterator mi = headers.begin(); mi != headers.end(); mi++)
        request.setRawHeader(mi.key(), mi.value());

    if ((msgType == MSG_TYPE_GET) ||
            (msgType == MSG_TYPE_NONE && data.isEmpty()))
    {
        EMIT_ERROR_ERR_IF ((_reply = _qnam->get(request)) == NULL,
                tr("Client failed creating GET request"));
    }
    else if ((msgType == MSG_TYPE_POST) ||
            (msgType == MSG_TYPE_NONE && !data.isEmpty()))
    {
        EMIT_ERROR_ERR_IF ((_reply = _qnam->post(request, data)) == NULL,
                tr("Client falied creating POST request"));
    } else {
        LIBENCLOUD_ERR ("bad msgType: " << QString::number(msgType));
    }

    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), 
            SLOT(_networkError(QNetworkReply::NetworkError)));

    _timer.start(LIBENCLOUD_CLIENT_TIMEOUT * 1000);
err:
    return;
}

//
// private slots
//

void Client::_proxyAuthenticationRequired (const QNetworkProxy &proxy, QAuthenticator *authenticator)
{ 
    LIBENCLOUD_UNUSED(authenticator); 
    LIBENCLOUD_UNUSED(proxy);

    EMIT_ERROR(tr("Proxy Authentication Required"));
}

void Client::_sslErrors (QNetworkReply *reply, const QList<QSslError> &errors) 
{ 
#ifndef Q_OS_WINCE
    QList<QSslError> ignoreErrors;

    foreach (QSslError err, errors)
    {
        switch (err.error())
        {
            case QSslError::SelfSignedCertificate:
            case QSslError::SelfSignedCertificateInChain:
            case QSslError::UnableToGetLocalIssuerCertificate:
            case QSslError::UnableToVerifyFirstCertificate:
                if (_verifyCA)
                {
                    CLIENT_DBG("[Client] CRITICAL QSslError (" << (int) err.error() << "): " << err.errorString());
                    LIBENCLOUD_EMIT(error(Error(Error::CodeServerVerifyFailed)));
                    _sslError = true;
                    break;
                }
                // else follow through/ignore
            case QSslError::HostNameMismatch:
                CLIENT_DBG("[Client] IGNORING QSslError (" << (int) err.error() << "): " << err.errorString()); 
                ignoreErrors.append(err);
                break;
            default:
                EMIT_ERROR("QSslError (" + QString::number(err.error()) + "): " + err.errorString()); 
                _sslError = true;
                break;

#if 0
            CLIENT_DBG("[Client] Peer Cert subj_CN=" << err.certificate().subjectInfo(QSslCertificate::CommonName) << \
                    " issuer_O=" << err.certificate().issuerInfo(QSslCertificate::Organization)); 
#endif
        }
    }

    reply->ignoreSslErrors(ignoreErrors);
#endif
}

void Client::_networkError (QNetworkReply::NetworkError err)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());
    QString extraMsg;
    QVariantMap json;
    bool ok;

    // get message from Switchboard
    extraMsg = reply->readAll();
    json = json::parse(extraMsg, ok).toMap();

    // default to errorString() if json error is not present or invalid
    if (extraMsg == "" || json.isEmpty() || !ok || json["error"].isNull())
        extraMsg = reply->errorString();
    else
        extraMsg = json["error"].toString();

    CLIENT_DBG("[Client] error: " << extraMsg);

    switch (err)
    {
        case QNetworkReply::UnknownContentError:
        case QNetworkReply::ProtocolFailure:
            LIBENCLOUD_EMIT(error(Error(Error::CodeServerError, extraMsg)));
            break;

        case QNetworkReply::SslHandshakeFailedError:

            // already emitted in _sslErrors()
            if (_sslError)
                break;

            // else follow through
        default:
            // url is too much detail for end user
            //LIBENCLOUD_EMIT(error(Error(Error::CodeServerUnreach, extraMsg + ", url: " + reply->url().toString())));
            LIBENCLOUD_EMIT(error(Error(Error::CodeServerUnreach, extraMsg)));
            break;
    }
}

/**
 * "Note: After the request has finished, it is the responsibility of the user
 * to delete the QNetworkReply object at an appropriate time. Do not directly
 * delete it inside the slot connected to finished()"
 */
void Client::_finished (QNetworkReply *reply) 
{ 

    _timer.stop();

    // Possible error code remappings (if required because they should not
    // occur with proper configuration):
    //  - handshake failed (6)
    //  - key values mismatch (99)
    if (reply->error())
    {
        LIBENCLOUD_DBG("[Client] Error in reply (" << reply->error() << "): " << reply->errorString());
        goto err;
    }

    _response = reply->readAll();

    CLIENT_DBG("[Client] ### <<<<< ### " << _response);

    reply->deleteLater();
    _reply = NULL;

    emit complete(_response);

    return;
err:
    reply->deleteLater();
    _reply = NULL;
}

void Client::_timeout ()
{
    LIBENCLOUD_TRACE;

    _reply->abort();
}

void Client::_connectQnam()
{
    connect(_qnam, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this,
            SLOT(_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
    connect(_qnam, SIGNAL(sslErrors(QNetworkReply *,QList<QSslError>)), this,
            SLOT(_sslErrors(QNetworkReply *,QList<QSslError>)));
    connect(_qnam, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(_finished(QNetworkReply *)));
}

} // namespace libencloud
