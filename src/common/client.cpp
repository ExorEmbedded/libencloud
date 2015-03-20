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
    : _verifyCA(true)
    , _debug(true)
    , _sslError(false)
{
    LIBENCLOUD_TRACE;
    
    connect(&_qnam, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this,
            SLOT(_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
    connect(&_qnam, SIGNAL(sslErrors(QNetworkReply *,QList<QSslError>)), this,
            SLOT(_sslErrors(QNetworkReply *,QList<QSslError>)));
    connect(&_qnam, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(_finished(QNetworkReply *)));
}

Client::~Client ()
{
    LIBENCLOUD_TRACE;
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
    QNetworkReply *reply = NULL;
    _sslError = false;

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
        EMIT_ERROR_ERR_IF ((reply = _qnam.get(request)) == NULL,
                tr("Client failed creating GET request"));
    }
    else if ((msgType == MSG_TYPE_POST) ||
            (msgType == MSG_TYPE_NONE && !data.isEmpty()))
    {
        EMIT_ERROR_ERR_IF ((reply = _qnam.post(request, data)) == NULL,
                tr("Client falied creating POST request"));
    } else {
        LIBENCLOUD_ERR ("bad msgType: " << QString::number(msgType));
    }

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), 
            SLOT(_networkError(QNetworkReply::NetworkError)));
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
            LIBENCLOUD_EMIT(error(Error(Error::CodeServerUnreach, extraMsg + ", url: " + reply->url().toString())));
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
    QString response;

    // Possible error code remappings (if required because they should not
    // occur with proper configuration):
    //  - handshake failed (6)
    //  - key values mismatch (99)

    LIBENCLOUD_ERR_MSG_IF (reply->error(),
            "[Client] Error in reply (" << reply->error() << "): " << reply->errorString());

    response = reply->readAll();

    CLIENT_DBG("[Client] ### <<<<< ### " << response);

    reply->deleteLater();

    emit complete(response);

    return;
err:
    reply->deleteLater();
}

} // namespace libencloud
