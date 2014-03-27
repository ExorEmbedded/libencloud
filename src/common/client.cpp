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
    : _debug (true)
{
    connect(&_qnam, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this,
            SLOT(_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
    connect(&_qnam, SIGNAL(sslErrors(QNetworkReply *,QList<QSslError>)), this,
            SLOT(_sslErrors(QNetworkReply *,QList<QSslError>)));
    connect(&_qnam, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(_finished(QNetworkReply *)));
}

void Client::setDebug (bool b) 
{
    _debug = b;
}

void Client::run (const QUrl &url, const QUrl &params, const QMap<QByteArray, QByteArray> &headers,
        const QSslConfiguration &conf)
{
    CLIENT_DBG("url: " << url.toString());
    CLIENT_DBG(" ### >>>>> ### " << params.toString());

    QNetworkRequest request(url);
    QNetworkReply *reply = NULL;

    request.setSslConfiguration(conf);
    request.setRawHeader("User-Agent", LIBENCLOUD_USERAGENT);
    request.setRawHeader("Host", url.host().toAscii());

    // override with passed custom headers
    for (QMap<QByteArray, QByteArray>::const_iterator mi = headers.begin(); mi != headers.end(); mi++)
        request.setRawHeader(mi.key(), mi.value());

    if (params.isEmpty())
    {
        EMIT_ERROR_ERR_IF ((reply = _qnam.get(request)) == NULL,
                tr("Client failed creating GET request"));
    }
    else
    {
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
        EMIT_ERROR_ERR_IF ((reply = _qnam.post(request, params.encodedQuery())) == NULL,
                tr("Client falied creating POST request"));
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
    const QNetworkProxy *p = &proxy;  //unused
    LIBENCLOUD_UNUSED(p);

    EMIT_ERROR(tr("Proxy Authentication Required"));
}

void Client::_sslErrors (QNetworkReply *reply, const QList<QSslError> &errors) 
{ 
    QList<QSslError> ignoreErrors;

    foreach (QSslError err, errors) 
    {
        switch (err.error())
        {
            case QSslError::SelfSignedCertificateInChain:
            case QSslError::HostNameMismatch:
                CLIENT_DBG("IGNORING QSslError (" << (int) err.error() << "): " << err.errorString()); 
                ignoreErrors.append(err);
                break;
            default:
                LIBENCLOUD_ERR("QSslError (" << (int) err.error() << "): " << err.errorString()); 
                break;

#if 0
            CLIENT_DBG("Peer Cert subj_CN=" << err.certificate().subjectInfo(QSslCertificate::CommonName) << \
                    " issuer_O=" << err.certificate().issuerInfo(QSslCertificate::Organization)); 
#endif
        }
    }

    reply->ignoreSslErrors(ignoreErrors);

    // Don't emit anything here - remainings error are caught in _networkError()
}

void Client::_networkError (QNetworkReply::NetworkError err)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());

    LIBENCLOUD_UNUSED(err);

    switch (reply->error())
    {
        default:
            LIBENCLOUD_EMIT(error(Error(Error::CodeServerUnreach)));
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
            "Error in reply (" << reply->error() << "): " << reply->errorString());

    response = reply->readAll();

    CLIENT_DBG(" ### <<<<< ### " << response);

    reply->deleteLater();

    emit complete(response);

    return;
err:
    reply->deleteLater();
}

} // namespace libencloud
