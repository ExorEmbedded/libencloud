#include "client.h"

namespace libencloud {

Client::Client ()
    : cfg(NULL), reply(NULL)
{
    LIBENCLOUD_TRACE;
}

Client::~Client ()
{
    LIBENCLOUD_TRACE;
}

libencloud_rc Client::setConfig (libencloud::Config *cfg)
{
    LIBENCLOUD_RETURN_IF (cfg == NULL, LIBENCLOUD_RC_BADPARAMS);

    this->cfg = cfg;

    return LIBENCLOUD_RC_SUCCESS;
}

libencloud_rc Client::run (libencloud::ProtocolType protocol, libencloud::Message &message)
{
    libencloud_rc rc;
    QUrl serviceURL;
    QUrl params;
    QSslConfiguration config;
    QString response, errString;

    // get proper configuration based on protocol type
    LIBENCLOUD_RETURN_IF (__loadSslConf(protocol, serviceURL, config), LIBENCLOUD_RC_BADPARAMS);

    LIBENCLOUD_ERR_IF ((rc = message.encodeRequest(serviceURL, params)));

    LIBENCLOUD_DBG("url=" << serviceURL);

    if ((rc = __run(serviceURL, params, config, response)))
        return rc;

    LIBENCLOUD_DBG(" ### <<<<< ###  " << response);

    LIBENCLOUD_ERR_IF ((rc = message.decodeResponse(response, errString)));

    return LIBENCLOUD_RC_SUCCESS;
err:
    if (!errString.isEmpty())
        LIBENCLOUD_ERR("SB error: " << errString);
    
    return rc;
}

libencloud_rc Client::__run (const QUrl &url, const QUrl &params, const QSslConfiguration &sslconf, QString &response)
{
    libencloud_rc rc = this->error = LIBENCLOUD_RC_SUCCESS;

    QNetworkAccessManager qnam;
    QNetworkRequest request(url);

    request.setSslConfiguration(sslconf);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", LIBENCLOUD_STRING);
    request.setRawHeader("X-Custom-User-Agent", LIBENCLOUD_STRING);

    if (url.path().compare(LIBENCLOUD_CMD_GETCONFIG) == 0)
        request.setRawHeader("Host", LIBENCLOUD_GETCONFIG_HOSTNAME);

    LIBENCLOUD_RETURN_IF ((this->loop = new QEventLoop) == NULL, LIBENCLOUD_RC_NOMEM);

    connect(&qnam, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this,
            SLOT(proxyAuthenticationRequiredSlot(const QNetworkProxy &, QAuthenticator *)));
    connect(&qnam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this,
            SLOT(sslErrorsSlot(QNetworkReply*,QList<QSslError>)));
    connect(&qnam, SIGNAL(finished(QNetworkReply*)), this->loop,
            SLOT(quit()));

    QTimer::singleShot(this->cfg->config.timeout, this, SLOT(timeoutSlot()));

    if (params.isEmpty())
        LIBENCLOUD_ERR_RC_IF ((this->reply = qnam.get(request)) == NULL, LIBENCLOUD_RC_FAILED);
    else
        LIBENCLOUD_ERR_RC_IF ((this->reply = qnam.post(request, params.encodedQuery())) == NULL, LIBENCLOUD_RC_FAILED);

    connect(this->reply, SIGNAL(error(QNetworkReply::NetworkError)), 
            SLOT(networkErrorSlot(QNetworkReply::NetworkError)));

    LIBENCLOUD_ERR_IF (this->loop->exec());

    // All NetworkError(s) currently handled by slot as LIBENCLOUD_RC_FAILED.
    // Possible error code remappings (if required because they should not
    // occur with proper configuration):
    //  - handshake failed (6)
    //  - key values mismatch (99)

    LIBENCLOUD_ERR_MSG_IF(this->error || reply->error(),
            "Error in reply (" << reply->error() << "): " << reply->errorString());
    response = reply->readAll();

    delete this->reply;
    this->reply = NULL;

    if (this->loop)
    {
        delete this->loop;
        this->loop = NULL;
    }

    return LIBENCLOUD_RC_SUCCESS;
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

    return (rc ? rc : LIBENCLOUD_RC_GENERIC);
}

void Client::proxyAuthenticationRequiredSlot (const QNetworkProxy &proxy, QAuthenticator *authenticator)
{ 
    LIBENCLOUD_UNUSED(authenticator); 
    const QNetworkProxy *p = &proxy;  //unused
    LIBENCLOUD_UNUSED(p);

    LIBENCLOUD_ERR(""); 

    this->error = LIBENCLOUD_RC_BADAUTH;
}

void Client::sslErrorsSlot (QNetworkReply *reply, const QList<QSslError> &errors) 
{ 
    LIBENCLOUD_UNUSED(reply);
    
    LIBENCLOUD_ERR(""); 

    // Ignore the SslError 22 "The host name did not match any of the valid hosts for this certificate"
    if ((errors.size() == 1) && (errors.first().error() == QSslError::HostNameMismatch)) {
        LIBENCLOUD_DBG("The host name did not match any of the valid hosts for this certificate");
        reply->ignoreSslErrors(errors);
        return;
    }

    this->error = LIBENCLOUD_RC_BADAUTH;

    foreach (QSslError err, errors) 
    {
        LIBENCLOUD_ERR("QSslError (" << (int) err.error() << "): " << err.errorString()); 
        LIBENCLOUD_DBG("Peer Cert subj_CN=" << err.certificate().subjectInfo(QSslCertificate::CommonName) << \
                " issuer_O=" << err.certificate().issuerInfo(QSslCertificate::Organization)); 
    }
}

void Client::networkErrorSlot (QNetworkReply::NetworkError err)
{ 
    LIBENCLOUD_UNUSED(err);

    LIBENCLOUD_ERR("NetworkError (" << err << ")");

    if (!this->error)  // can be already set by timeoutSlot()
        this->error = LIBENCLOUD_RC_FAILED;
}

void Client::timeoutSlot ()
{ 
    LIBENCLOUD_TRACE; 

    this->error = LIBENCLOUD_RC_TIMEOUT;

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
    LIBENCLOUD_UNUSED(reply);

    LIBENCLOUD_TRACE; 
}

libencloud_rc Client::__loadSslConf (libencloud::ProtocolType protocol, QUrl &url, QSslConfiguration &sslconf)
{
    libencloud_config_ssl_t *sslcfg;
    
    LIBENCLOUD_RETURN_IF (this->cfg == NULL, LIBENCLOUD_RC_BADPARAMS);

    switch (protocol) {
        case ProtocolTypeInit:
            sslcfg = &this->cfg->config.sslInit;
            break;
        case ProtocolTypeOp:
            sslcfg = &this->cfg->config.sslOp;
            break;
        default:
            LIBENCLOUD_RETURN_IF (1, LIBENCLOUD_RC_BADPARAMS);
    }

    url = sslcfg->sbUrl;
    LIBENCLOUD_DBG("url=" << url.toString());

    // get CA cert(s)
    QSslCertificate ca;
    QList<QSslCertificate> cas(ca.fromPath(sslcfg->caPath.absoluteFilePath()));
    LIBENCLOUD_RETURN_MSG_IF (cas.empty(), LIBENCLOUD_RC_BADPARAMS, "missing CA cert!");
    LIBENCLOUD_DBG("CaCert subj_CN=" << cas.first().subjectInfo(QSslCertificate::CommonName) << \
            " issuer_O=" << cas.first().issuerInfo(QSslCertificate::Organization));

    // get local cert(s)
    QSslCertificate cert;
    QList<QSslCertificate> certs(cert.fromPath(sslcfg->certPath.absoluteFilePath()));
    LIBENCLOUD_RETURN_MSG_IF (certs.empty(), LIBENCLOUD_RC_BADPARAMS, "missing cert!");
    LIBENCLOUD_DBG("Cert subj_CN=" << certs.first().subjectInfo(QSslCertificate::CommonName) << \
            " issuer_O=" << certs.first().issuerInfo(QSslCertificate::Organization));

    // get local key
    QFile kfile(sslcfg->keyPath.absoluteFilePath());
    kfile.open(QIODevice::ReadOnly | QIODevice::Text);
    QSslKey key(&kfile, QSsl::Rsa);
    LIBENCLOUD_RETURN_MSG_IF (key.isNull(), LIBENCLOUD_RC_BADPARAMS, "missing key!");
    LIBENCLOUD_DBG("Key length=" << key.length() << " algorithm=" << key.algorithm());

    sslconf.setCaCertificates(cas);
    sslconf.setLocalCertificate(certs.first());
    sslconf.setPrivateKey(key);

    return LIBENCLOUD_RC_SUCCESS;
}

void Client::timerTimeout ()
{
    LIBENCLOUD_TRACE;
}

} // namespace libencloud
