#include "client.h"

namespace encloud {

Client::Client ()
    : cfg(NULL), reply(NULL)
{
    ENCLOUD_TRACE;
}

Client::~Client ()
{
    ENCLOUD_TRACE;
}

encloud_rc Client::setConfig (encloud::Config *cfg)
{
    ENCLOUD_RETURN_IF (cfg == NULL, ENCLOUD_RC_BADPARAMS);

    this->cfg = cfg;

    return ENCLOUD_RC_SUCCESS;
}

encloud_rc Client::run (encloud::ProtocolType protocol, encloud::Message &message)
{
    encloud_rc rc;
    QUrl serviceURL;
    QUrl params;
    QSslConfiguration config;
    QString response, errString;

    // get proper configuration based on protocol type
    ENCLOUD_RETURN_IF (__loadSslConf(protocol, serviceURL, config), ENCLOUD_RC_BADPARAMS);

    ENCLOUD_ERR_IF ((rc = message.encodeRequest(serviceURL, params)));

    ENCLOUD_DBG("url=" << serviceURL);

    if ((rc = __run(serviceURL, params, config, response)))
        return rc;

    ENCLOUD_DBG(" ### <<<<< ###  " << response);

    ENCLOUD_ERR_IF ((rc = message.decodeResponse(response, errString)));

    return ENCLOUD_RC_SUCCESS;
err:
    if (!errString.isEmpty())
        ENCLOUD_ERR("SB error: " << errString);
    
    return rc;
}

encloud_rc Client::__run (const QUrl &url, const QUrl &params, const QSslConfiguration &sslconf, QString &response)
{
    encloud_rc rc = this->error = ENCLOUD_RC_SUCCESS;

    QNetworkAccessManager qnam;
    QNetworkRequest request(url);

    request.setSslConfiguration(sslconf);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", ENCLOUD_STRING);
    request.setRawHeader("X-Custom-User-Agent", ENCLOUD_STRING);

    if (url.path().compare(ENCLOUD_CMD_GETCONFIG) == 0)
        request.setRawHeader("Host", ENCLOUD_GETCONFIG_HOSTNAME);

    ENCLOUD_RETURN_IF ((this->loop = new QEventLoop) == NULL, ENCLOUD_RC_NOMEM);

    connect(&qnam, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this,
            SLOT(proxyAuthenticationRequiredSlot(const QNetworkProxy &, QAuthenticator *)));
    connect(&qnam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this,
            SLOT(sslErrorsSlot(QNetworkReply*,QList<QSslError>)));
    connect(&qnam, SIGNAL(finished(QNetworkReply*)), this->loop,
            SLOT(quit()));

    QTimer::singleShot(this->cfg->config.timeout, this, SLOT(timeoutSlot()));

    if (params.isEmpty())
        ENCLOUD_ERR_RC_IF ((this->reply = qnam.get(request)) == NULL, ENCLOUD_RC_FAILED);
    else
        ENCLOUD_ERR_RC_IF ((this->reply = qnam.post(request, params.encodedQuery())) == NULL, ENCLOUD_RC_FAILED);

    connect(this->reply, SIGNAL(error(QNetworkReply::NetworkError)), 
            SLOT(networkErrorSlot(QNetworkReply::NetworkError)));

    ENCLOUD_ERR_IF (this->loop->exec());

    // All NetworkError(s) currently handled by slot as ENCLOUD_RC_FAILED.
    // Possible error code remappings (if required because they should not
    // occur with proper configuration):
    //  - handshake failed (6)
    //  - key values mismatch (99)

    ENCLOUD_ERR_MSG_IF(this->error || reply->error(),
            "Error in reply (" << reply->error() << "): " << reply->errorString());
    response = reply->readAll();

    delete this->reply;
    this->reply = NULL;

    if (this->loop)
    {
        delete this->loop;
        this->loop = NULL;
    }

    return ENCLOUD_RC_SUCCESS;
err:
    if (this->reply)
    {
        delete this->reply;
        this->reply = NULL;
    }

    if (this->loop)
    {
        delete this->loop;
        this->loop = NULL;
    }

    if (this->error)
        return this->error;

    return (rc ? rc : ENCLOUD_RC_GENERIC);
}

void Client::proxyAuthenticationRequiredSlot (const QNetworkProxy &proxy, QAuthenticator *authenticator)
{ 
    ENCLOUD_UNUSED(authenticator); 
    const QNetworkProxy *p = &proxy;  //unused
    ENCLOUD_UNUSED(p);

    ENCLOUD_ERR(""); 

    this->error = ENCLOUD_RC_BADAUTH;
}

void Client::sslErrorsSlot (QNetworkReply *reply, const QList<QSslError> &errors) 
{ 
    ENCLOUD_UNUSED(reply);
    
    ENCLOUD_ERR(""); 

    // Ignore the SslError 22 "The host name did not match any of the valid hosts for this certificate"
    if ((errors.size() == 1) && (errors.first().error() == QSslError::HostNameMismatch)) {
        ENCLOUD_DBG("The host name did not match any of the valid hosts for this certificate");
        reply->ignoreSslErrors(errors);
        return;
    }

    this->error = ENCLOUD_RC_BADAUTH;

    foreach (QSslError err, errors) 
    {
        ENCLOUD_ERR("QSslError (" << (int) err.error() << "): " << err.errorString()); 
        ENCLOUD_DBG("Peer Cert subj_CN=" << err.certificate().subjectInfo(QSslCertificate::CommonName) << \
                " issuer_O=" << err.certificate().issuerInfo(QSslCertificate::Organization)); 
    }
}

void Client::networkErrorSlot (QNetworkReply::NetworkError err)
{ 
    ENCLOUD_UNUSED(err);

    ENCLOUD_ERR("NetworkError (" << err << ")");

    if (!this->error)  // can be already set by timeoutSlot()
        this->error = ENCLOUD_RC_FAILED;
}

void Client::timeoutSlot ()
{ 
    ENCLOUD_TRACE; 

    this->error = ENCLOUD_RC_TIMEOUT;

    if (this->reply)
        this->reply->abort();

    if (this->loop)
    {
        this->loop->quit();
        delete this->loop;
        this->loop = NULL;
    }
}

/**
 * "Note: After the request has finished, it is the responsibility of the user
 * to delete the QNetworkReply object at an appropriate time. Do not directly
 * delete it inside the slot connected to finished()"
 */
void Client::finishedSlot (QNetworkReply *reply) 
{ 
    ENCLOUD_UNUSED(reply);

    ENCLOUD_TRACE; 
}

encloud_rc Client::__loadSslConf (encloud::ProtocolType protocol, QUrl &url, QSslConfiguration &sslconf)
{
    encloud_config_ssl_t *sslcfg;
    
    ENCLOUD_RETURN_IF (this->cfg == NULL, ENCLOUD_RC_BADPARAMS);

    switch (protocol) {
        case ProtocolTypeInit:
            sslcfg = &this->cfg->config.sslInit;
            break;
        case ProtocolTypeOp:
            sslcfg = &this->cfg->config.sslOp;
            break;
        default:
            ENCLOUD_RETURN_IF (1, ENCLOUD_RC_BADPARAMS);
    }

    url = sslcfg->sbUrl;
    ENCLOUD_DBG("url=" << url.toString());

    // get CA cert(s)
    QSslCertificate ca;
    QList<QSslCertificate> cas(ca.fromPath(sslcfg->caPath.absoluteFilePath()));
    ENCLOUD_RETURN_MSG_IF (cas.empty(), ENCLOUD_RC_BADPARAMS, "missing CA cert!");
    ENCLOUD_DBG("CaCert subj_CN=" << cas.first().subjectInfo(QSslCertificate::CommonName) << \
            " issuer_O=" << cas.first().issuerInfo(QSslCertificate::Organization));

    // get local cert(s)
    QSslCertificate cert;
    QList<QSslCertificate> certs(cert.fromPath(sslcfg->certPath.absoluteFilePath()));
    ENCLOUD_RETURN_MSG_IF (certs.empty(), ENCLOUD_RC_BADPARAMS, "missing cert!");
    ENCLOUD_DBG("Cert subj_CN=" << certs.first().subjectInfo(QSslCertificate::CommonName) << \
            " issuer_O=" << certs.first().issuerInfo(QSslCertificate::Organization));

    // get local key
    QFile kfile(sslcfg->keyPath.absoluteFilePath());
    kfile.open(QIODevice::ReadOnly | QIODevice::Text);
    QSslKey key(&kfile, QSsl::Rsa);
    ENCLOUD_RETURN_MSG_IF (key.isNull(), ENCLOUD_RC_BADPARAMS, "missing key!");
    ENCLOUD_DBG("Key length=" << key.length() << " algorithm=" << key.algorithm());

    sslconf.setCaCertificates(cas);
    sslconf.setLocalCertificate(certs.first());
    sslconf.setPrivateKey(key);

    return ENCLOUD_RC_SUCCESS;
}

void Client::timerTimeout ()
{
    ENCLOUD_TRACE;
}

} // namespace encloud
