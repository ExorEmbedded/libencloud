#include <common/common.h>
#include <common/config.h>
#include <common/client.h>

#define EMIT_ERROR(msg) LIBENCLOUD_EMIT(error(msg))
#define EMIT_ERROR_ERR_IF(cond, msg) LIBENCLOUD_EMIT_ERR_IF(cond, error(msg))

namespace libencloud {

Client::Client ()
{
    connect(&_qnam, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this,
            SLOT(_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
    connect(&_qnam, SIGNAL(sslErrors(QNetworkReply *,QList<QSslError>)), this,
            SLOT(_sslErrors(QNetworkReply *,QList<QSslError>)));
    connect(&_qnam, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(_finished(QNetworkReply *)));
}

void Client::run (const QUrl &url, const QUrl &params, const QMap<QByteArray, QByteArray> &headers,
        const QSslConfiguration &conf)
{
    LIBENCLOUD_DBG("url: " << url.toString());
    LIBENCLOUD_DBG(" ### >>>>> ### " << params.toString());

    QNetworkRequest request(url);
    QNetworkReply *reply = NULL;

    request.setSslConfiguration(conf);
    request.setRawHeader("User-Agent", LIBENCLOUD_USERAGENT);
    request.setRawHeader("X-Custom-User-Agent", LIBENCLOUD_USERAGENT);
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
                LIBENCLOUD_DBG("IGNORING QSslError (" << (int) err.error() << "): " << err.errorString()); 
                ignoreErrors.append(err);
                break;
            default:
                LIBENCLOUD_ERR("QSslError (" << (int) err.error() << "): " << err.errorString()); 
                break;

#if 0
            LIBENCLOUD_DBG("Peer Cert subj_CN=" << err.certificate().subjectInfo(QSslCertificate::CommonName) << \
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

    EMIT_ERROR(reply->errorString());
    
    reply->deleteLater();
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

    LIBENCLOUD_DBG(" ### <<<<< ### " << response);

    reply->deleteLater();

    emit complete(response);

    return;
err:
    reply->deleteLater();
}

} // namespace libencloud
