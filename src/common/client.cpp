#include <common/common.h>
#include <common/config.h>
#include <common/client.h>

#define SIGNAL_ERR_IF(cond) LIBENCLOUD_EMIT_ERR_IF(cond, error())

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

void Client::run (const QUrl &url, const QUrl &params, const QSslConfiguration &conf)
{
    LIBENCLOUD_DBG("url: " << url.toString());
    LIBENCLOUD_DBG(" ### >>>>> ### " << params.toString());

    QNetworkRequest request(url);
    QNetworkReply *reply = NULL;

    request.setSslConfiguration(conf);
    request.setRawHeader("User-Agent", LIBENCLOUD_STRING);
    request.setRawHeader("X-Custom-User-Agent", LIBENCLOUD_STRING);

    if (url.path().compare(LIBENCLOUD_CMD_GETCONFIG) == 0)
        request.setRawHeader("Host", LIBENCLOUD_GETCONFIG_HOSTNAME);

    if (params.isEmpty())
    {
        LIBENCLOUD_ERR_IF ((reply = _qnam.get(request)) == NULL);
    }
    else
    {
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
        LIBENCLOUD_ERR_IF ((reply = _qnam.post(request, params.encodedQuery())) == NULL);
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

    LIBENCLOUD_ERR(""); 

    emit error();
}

void Client::_sslErrors (QNetworkReply *reply, const QList<QSslError> &errors) 
{ 
    LIBENCLOUD_UNUSED(reply);
    
    LIBENCLOUD_ERR(""); 

    // Ignore the SslError 22 "The host name did not match any of the valid hosts for this certificate"
    if ((errors.size() == 1) && (errors.first().error() == QSslError::HostNameMismatch)) {
        LIBENCLOUD_DBG("The host name did not match any of the valid hosts for this certificate");
        reply->ignoreSslErrors(errors);
        return;
    }

    emit error();

    foreach (QSslError err, errors) 
    {
        LIBENCLOUD_ERR("QSslError (" << (int) err.error() << "): " << err.errorString()); 
        LIBENCLOUD_DBG("Peer Cert subj_CN=" << err.certificate().subjectInfo(QSslCertificate::CommonName) << \
                " issuer_O=" << err.certificate().issuerInfo(QSslCertificate::Organization)); 
    }
}

void Client::_networkError (QNetworkReply::NetworkError err)
{ 
    LIBENCLOUD_UNUSED(err);

    LIBENCLOUD_ERR("NetworkError (" << err << ")");

    emit error();
}

/**
 * "Note: After the request has finished, it is the responsibility of the user
 * to delete the QNetworkReply object at an appropriate time. Do not directly
 * delete it inside the slot connected to finished()"
 */
void Client::_finished (QNetworkReply *reply) 
{ 
    LIBENCLOUD_TRACE; 

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
    emit error();
}

} // namespace libencloud
