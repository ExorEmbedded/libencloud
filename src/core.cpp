#include "core.h"

/** 
 * \brief Create a new LIBENCLOUD object
 *
 * The object is returned in 'plibencloud' and must be destroyed by caller using libencloud_destroy().
 *
 * If no running instance of QCoreApplication is detected (libencloud used as a standalone library),
 * an internal application is created passing {argc, argv} command-line arguments (optional).
 */
libencloud_rc libencloud_create (int argc, char *argv[], libencloud_t **plibencloud)
{
    libencloud_rc rc = LIBENCLOUD_RC_SUCCESS;

    libencloud_t *libencloud = NULL; 

    LIBENCLOUD_TRACE;
    LIBENCLOUD_RETURN_IF (plibencloud == NULL, LIBENCLOUD_RC_BADPARAMS);

    LIBENCLOUD_RETURN_IF ((libencloud = (libencloud_t *) calloc(1, sizeof(libencloud_t))) == NULL, LIBENCLOUD_RC_NOMEM);
    LIBENCLOUD_ERR_RC_IF ((libencloud->context = new libencloud::Context()) == NULL, LIBENCLOUD_RC_NOMEM);
    LIBENCLOUD_ERR_RC_IF ((rc = libencloud->context->init(argc, argv)), rc);

    *plibencloud = libencloud;

    return LIBENCLOUD_RC_SUCCESS;
err:
    if (libencloud)
        libencloud_destroy(libencloud);

    return (rc ? rc : LIBENCLOUD_RC_GENERIC);
}

/* \brief Dispose of 'libencloud' object */
libencloud_rc libencloud_destroy (libencloud_t *libencloud)
{
    LIBENCLOUD_TRACE;
    LIBENCLOUD_RETURN_IF (libencloud == NULL, LIBENCLOUD_RC_BADPARAMS);

    LIBENCLOUD_DELETE(libencloud->context);
    LIBENCLOUD_FREE(libencloud);

    return LIBENCLOUD_RC_SUCCESS;
}

libencloud_rc libencloud_set_state_cb (libencloud_t *libencloud, libencloud_state_cb state_cb, void *arg)
{
    LIBENCLOUD_TRACE;
    LIBENCLOUD_RETURN_IF (libencloud == NULL, LIBENCLOUD_RC_BADPARAMS);
    LIBENCLOUD_RETURN_IF (state_cb == NULL, LIBENCLOUD_RC_BADPARAMS);

    return libencloud->context->setStateCb(state_cb, arg);
}

libencloud_rc libencloud_start (libencloud_t *libencloud)
{
    LIBENCLOUD_TRACE;
    LIBENCLOUD_RETURN_IF (libencloud == NULL, LIBENCLOUD_RC_BADPARAMS);

    return libencloud->context->start();
}

libencloud_rc libencloud_stop (libencloud_t *libencloud)
{
    LIBENCLOUD_TRACE;
    LIBENCLOUD_RETURN_IF (libencloud == NULL, LIBENCLOUD_RC_BADPARAMS);

    return libencloud->context->stop();
}

#if 0
/**
 *  \brief Synchronous retrieval of info from SB
 *
 * The request is performed upon the Initialization secure channel set up using the
 * {init_key, init_cert} pair installed during production phase.
 * 
 * The license read from persistent storage along with retrieved hardware info are sent
 * as proof of identity.
 * 
 * License info is returned to the user and CSR template is stored internally.
 * 
 * Note: 'pinfo' points to an internal object (no memory management nlibencloudssary).
 */
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_retr_info (libencloud_t *libencloud, libencloud_info_t **pinfo)
{
    LIBENCLOUD_TRACE;
    LIBENCLOUD_RETURN_IF (libencloud == NULL, LIBENCLOUD_RC_BADPARAMS);
    LIBENCLOUD_RETURN_IF (pinfo == NULL, LIBENCLOUD_RC_BADPARAMS);

    libencloud_rc rc = LIBENCLOUD_RC_GENERIC;
    libencloud::MessageRetrInfo msg;
    QString csrfn = libencloud->cfg->config.csrTmplPath.absoluteFilePath();
    QFile csrf(csrfn);
    QString cafn = libencloud->cfg->config.sslOp.caPath.absoluteFilePath();
    QFile caf(cafn);

    bool ok;

#ifdef LIBENCLOUD_TYPE_SECE
    msg.license = QUuid(libencloud->cfg->settings->value("lic").toString());
    LIBENCLOUD_RETURN_IF (msg.license.isNull(), LIBENCLOUD_RC_NOLICENSE);

    msg.hwInfo = libencloud::utils::getHwInfo();
#endif

    LIBENCLOUD_ERR_RC_IF ((rc = libencloud->client->run(libencloud::ProtocolTypeInit, msg)), rc);

    libencloud->info.license_valid = msg.valid;
    libencloud->info.license_expiry = msg.expiry.toTime_t();

    // save the CSR template to file
    LIBENCLOUD_ERR_RC_IF (!csrf.open(QIODevice::WriteOnly), LIBENCLOUD_RC_SYSERR);
    LIBENCLOUD_ERR_RC_IF (csrf.write(libencloud::json::serialize(msg.csrTmpl, ok).toAscii()) == -1, LIBENCLOUD_RC_SYSERR);
    csrf.close();

    // save the Operation CA certificate to file
    LIBENCLOUD_ERR_RC_IF (!caf.open(QIODevice::WriteOnly), LIBENCLOUD_RC_SYSERR);
    LIBENCLOUD_ERR_RC_IF (caf.write(msg.caCert.toPem()) == -1, LIBENCLOUD_RC_SYSERR);
    caf.close();

    *pinfo = &libencloud->info;

    rc = LIBENCLOUD_RC_SUCCESS;
err:
    if (rc) {
        csrf.close();
        caf.close();
    }

    return rc;
}

/**
 *  \brief Synchronous retrieval of certificate from SB
 *
 * The request is performed upon the Initialization secure channel set up using the
 * {init_key, init_cert} pair installed during production phase.
 * 
 * The license read from persistent storage along with retrieved hardware info are sent
 * as proof of identity. An internal key is generated and used to generate a Certificate
 * Signing Request which is sent to SB. 
 * 
 * Upon success, a new certificate is returned and stored internally.
 */
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_retr_sb_cert (libencloud_t *libencloud)
{
    LIBENCLOUD_TRACE;
    LIBENCLOUD_RETURN_IF (libencloud == NULL, LIBENCLOUD_RC_BADPARAMS);

    libencloud_rc rc = LIBENCLOUD_RC_GENERIC;
    libencloud::MessageRetrCert msg;
    char *buf = NULL;
    long len;
    QString keyfn = libencloud->cfg->config.sslOp.keyPath.absoluteFilePath();
    QString tmpkeyfn = keyfn + ".tmp";
    QFile tmpkey(tmpkeyfn);
    QString certfn = libencloud->cfg->config.sslOp.certPath.absoluteFilePath();
    QFile certfile(certfn);

#ifdef LIBENCLOUD_TYPE_SECE
    msg.license = QUuid(libencloud->cfg->settings->value("lic").toString());
    LIBENCLOUD_RETURN_IF (msg.license.isNull(), LIBENCLOUD_RC_NOLICENSE);

    msg.hwInfo = libencloud::utils::getHwInfo();
#endif

    // generate temporary key and CSR
    LIBENCLOUD_ERR_IF (libencloud_crypto_genkey(&libencloud->crypto, libencloud->cfg->config.rsaBits, qPrintable(tmpkeyfn)));
    LIBENCLOUD_ERR_IF (libencloud_crypto_gencsr(&libencloud->crypto, qPrintable(tmpkeyfn), &buf, &len));

    msg.csr = QByteArray(buf, len);

    LIBENCLOUD_ERR_RC_IF ((rc = libencloud->client->run(libencloud::ProtocolTypeInit, msg)), rc);

    // save the rlibencloudived certificate
    LIBENCLOUD_ERR_RC_IF (!certfile.open(QIODevice::WriteOnly), LIBENCLOUD_RC_SYSERR); 
    LIBENCLOUD_ERR_RC_IF (certfile.write(msg.cert.toPem()) == -1, LIBENCLOUD_RC_SYSERR);
    certfile.close();

    // all ok - now we can commit the temporary key
    QFile::remove(keyfn);
    LIBENCLOUD_RETURN_IF (!tmpkey.setPermissions(QFile::ReadOwner|QFile::WriteOwner), LIBENCLOUD_RC_SYSERR);
    LIBENCLOUD_ERR_RC_IF (!tmpkey.rename(keyfn), LIBENCLOUD_RC_SYSERR);

    rc = LIBENCLOUD_RC_SUCCESS;
err:
    if (buf)
        free(buf);
    return rc;
}

/**
 *  \brief Synchronous retrieval of configuration from SB
 *
 * The request is performed upon the Operation secure channel set up using the
 * {op_key, op_cert} pair installed via libencloud_retr_sb_cert().
 *
 * Upon success, configuration info is returned to the user in 'pconf'.
 * 
 * Note: 'pconf' points to an internal object (no memory management nlibencloudssary).
 */
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_retr_conf (libencloud_t *libencloud, libencloud_vpn_conf_t **pconf)
{
    libencloud_rc rc = LIBENCLOUD_RC_SUCCESS;
    libencloud::MessageRetrConf msg;

    LIBENCLOUD_TRACE;
    LIBENCLOUD_RETURN_IF (libencloud == NULL, LIBENCLOUD_RC_BADPARAMS);
    LIBENCLOUD_RETURN_IF (pconf == NULL, LIBENCLOUD_RC_BADPARAMS);

    LIBENCLOUD_RETURN_IF ((rc = libencloud->client->run(libencloud::ProtocolTypeOp, msg)), rc);

    strcpy(libencloud->conf.vpn_ip, qPrintable(msg.vpnIp));
    libencloud->conf.vpn_port = msg.vpnPort;
    strcpy(libencloud->conf.vpn_proto, qPrintable(msg.vpnProto));
    strcpy(libencloud->conf.vpn_type, qPrintable(msg.vpnType));

    *pconf = &libencloud->conf;

    return rc;
}

/** \brief Is license valid? */
LIBENCLOUD_DLLSPEC bool libencloud_info_get_license_valid (libencloud_info_t *info)
{
    LIBENCLOUD_RETURN_IF (info == NULL, false);

    return info->license_valid;
}

/** \brief Get certificate expiry - expressed in seconds since Epoch */
LIBENCLOUD_DLLSPEC time_t libencloud_info_get_license_expiry (libencloud_info_t *info)
{
    LIBENCLOUD_RETURN_IF (info == NULL, -1);

    return info->license_expiry;
}
#endif

/** \brief Return LIBENCLOUD version string */
LIBENCLOUD_DLLSPEC const char *libencloud_version ()
{
    return LIBENCLOUD_VERSION;
}

/** \brief Return LIBENCLOUD revision (GIT commit ID) */
LIBENCLOUD_DLLSPEC const char *libencloud_revision ()
{
    return LIBENCLOUD_REVISION;
}

/** \brief Convert a return code to string representation */
LIBENCLOUD_DLLSPEC const char *libencloud_strerror (libencloud_rc rc)
{
    switch (rc)
    {
        case LIBENCLOUD_RC_SUCCESS:
            return "Success";
        case LIBENCLOUD_RC_BADPARAMS:
            return "Bad parameters";
        case LIBENCLOUD_RC_NOMEM:
            return "Out of memory";
        case LIBENCLOUD_RC_BADCONFIG:
            return "Bad configuration";
        case LIBENCLOUD_RC_NOLICENSE:
            return "No license found";
        case LIBENCLOUD_RC_CONNECT:
            return "Connection error";
        case LIBENCLOUD_RC_BADAUTH:
            return "Authentication failed";
        case LIBENCLOUD_RC_TIMEOUT:
            return "Operation timed out";
        case LIBENCLOUD_RC_BADRESPONSE:
            return "Bad response from peer";
        case LIBENCLOUD_RC_INVALIDCERT:
            return "Certificate validity check failed";
        case LIBENCLOUD_RC_FAILED:
            return "Operation failed";
        case LIBENCLOUD_RC_SYSERR:
            return "System error";
        case LIBENCLOUD_RC_GENERIC:
            return "Generic error";
    }

    return "<undefined>";
}

