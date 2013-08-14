#include "client.h"

namespace Ece {

Client::Client ()
    : cfg(NULL), reply(NULL)
{
    ECE_TRACE;
}

Client::~Client ()
{
    ECE_TRACE;
}

ece_rc_t Client::setConfig (Ece::Config *cfg)
{
    ECE_RETURN_IF (cfg == NULL, ECE_RC_BADPARAMS);

    this->cfg = cfg;

    return ECE_RC_SUCCESS;
}

ece_rc_t Client::run (Ece::ProtocolType protocol, Ece::Message &message)
{
    ece_rc_t rc;
    QUrl serviceURL;
    QUrl params;
    QSslConfiguration config;
    QString response;

    // get proper configuration based on protocol type
    ECE_RETURN_IF (__loadSslConf(protocol, serviceURL, config), ECE_RC_BADPARAMS);

    ECE_ERR_IF ((rc = message.encodeRequest(serviceURL, params)));

    ECE_DBG("url=" << serviceURL);

    if ((rc = __run(serviceURL, params, config, response)))
        return rc;

    ECE_DBG(" *** < *** " << response);

    ECE_ERR_IF ((rc = message.decodeResponse(response)));

    return ECE_RC_SUCCESS;
err:
    return rc;
}

ece_rc_t Client::__run (const QUrl &url, const QUrl &params, const QSslConfiguration &sslconf, QString &response)
{
    ece_rc_t rc = this->error = ECE_RC_SUCCESS;

    QNetworkAccessManager qnam;
    QNetworkRequest request(url);

    request.setSslConfiguration(sslconf);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("User-Agent", ECE_STRING);
    request.setRawHeader("X-Custom-User-Agent", ECE_STRING);

    ECE_RETURN_IF ((this->loop = new QEventLoop) == NULL, ECE_RC_NOMEM);

    connect(&qnam, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this,
            SLOT(proxyAuthenticationRequiredSlot(const QNetworkProxy &, QAuthenticator *)));
    connect(&qnam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this,
            SLOT(sslErrorsSlot(QNetworkReply*,QList<QSslError>)));
    connect(&qnam, SIGNAL(finished(QNetworkReply*)), this->loop,
            SLOT(quit()));

    QTimer::singleShot(ECE_TIMEOUT, this, SLOT(timeoutSlot()));

    ECE_ERR_RC_IF ((this->reply = qnam.post(request, params.encodedQuery())) == NULL, ECE_RC_FAILED);

    connect(this->reply, SIGNAL(error(QNetworkReply::NetworkError)), 
            SLOT(networkErrorSlot(QNetworkReply::NetworkError)));

    ECE_ERR_IF (this->loop->exec());
    ECE_ERR_IF (this->error != ECE_RC_SUCCESS);

    ECE_ERR_MSG_IF(reply->error(),
            "Error in reply (" << reply->error() << "): " << reply->errorString());
    response = reply->readAll();

    delete this->reply;

    return ECE_RC_SUCCESS;
err:
    if (this->reply)
        delete this->reply;

    if (this->error)
        return this->error;

    return (rc ? rc : ECE_RC_GENERIC);
}

void Client::proxyAuthenticationRequiredSlot (const QNetworkProxy &proxy, QAuthenticator *authenticator)
{ 
    ECE_ERR(""); 

    this->error = ECE_RC_BADAUTH;
}

void Client::sslErrorsSlot (QNetworkReply *reply, const QList<QSslError> &errors) 
{ 
    ECE_ERR(""); 
    
    this->error = ECE_RC_BADAUTH;

    foreach (QSslError err, errors) {
        ECE_ERR("QSslError (" << (int) err.error() << "): " << err.errorString()); 
        ECE_DBG("Peer Cert subj_CN=" << err.certificate().subjectInfo(QSslCertificate::CommonName) << \
                " issuer_O=" << err.certificate().issuerInfo(QSslCertificate::Organization)); 
    }
}

void Client::networkErrorSlot (QNetworkReply::NetworkError err)
{ 
    ECE_DBG("NetworkError (" << err << ")");

    this->error = ECE_RC_FAILED;
}

void Client::timeoutSlot ()
{ 
    ECE_TRACE; 

    this->error = ECE_RC_TIMEOUT;
    this->reply->abort();
    this->loop->quit();
}

/**
 * "Note: After the request has finished, it is the responsibility of the user
 * to delete the QNetworkReply object at an appropriate time. Do not directly
 * delete it inside the slot connected to finished(). You can use the
 * deleteLater() function."
 */
void Client::finishedSlot (QNetworkReply *reply) 
{ 
    ECE_TRACE; 
}

ece_rc_t Client::__loadSslConf (Ece::ProtocolType protocol, QUrl &url, QSslConfiguration &sslconf)
{
    ece_config_ssl_t *sslcfg;
    
    ECE_RETURN_IF (this->cfg == NULL, ECE_RC_BADPARAMS);

    switch (protocol) {
        case ProtocolTypeInit:
            sslcfg = &this->cfg->config.sslInit;
            break;
        case ProtocolTypeOp:
            sslcfg = &this->cfg->config.sslOp;
            break;
        default:
            ECE_RETURN_IF (1, ECE_RC_BADPARAMS);
    }

    url = sslcfg->sbUrl;
    ECE_DBG("url=" << url.toString());

    // get CA cert(s)
    QSslCertificate cacert;
    QList<QSslCertificate> cacerts(cacert.fromPath(sslcfg->cacertPath.absoluteFilePath()));
    ECE_RETURN_MSG_IF (cacerts.empty(), ECE_RC_BADPARAMS, "missing CA cert!");
    ECE_DBG("CaCert subj_CN=" << cacerts.first().subjectInfo(QSslCertificate::CommonName) << \
            " issuer_O=" << cacerts.first().issuerInfo(QSslCertificate::Organization));

    // get local cert(s)
    QSslCertificate cert;
    QList<QSslCertificate> certs(cert.fromPath(sslcfg->certPath.absoluteFilePath()));
    ECE_RETURN_MSG_IF (certs.empty(), ECE_RC_BADPARAMS, "missing cert!");
    ECE_DBG("Cert subj_CN=" << certs.first().subjectInfo(QSslCertificate::CommonName) << \
            " issuer_O=" << certs.first().issuerInfo(QSslCertificate::Organization));

    // get local key
    QFile kfile(sslcfg->keyPath.absoluteFilePath());
    kfile.open(QIODevice::ReadOnly | QIODevice::Text);
    QSslKey key(&kfile, QSsl::Rsa);
    ECE_RETURN_MSG_IF (key.isNull(), ECE_RC_BADPARAMS, "missing key!");
    ECE_DBG("Key length=" << key.length() << " algorithm=" << key.algorithm());

    sslconf.setCaCertificates(cacerts);
    sslconf.setLocalCertificate(certs.first());
    sslconf.setPrivateKey(key);

    return ECE_RC_SUCCESS;
}

} // namespace Ece
