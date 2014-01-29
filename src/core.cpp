#include "core.h"

/** 
 * \brief Create a new ENCLOUD object
 *
 * The object is returned in 'pencloud' and must be destroyed by caller using encloud_destroy().
 *
 * If no running instance of QCoreApplication is detected (libencloud used as a standalone library),
 * an internal application is created passing {argc, argv} command-line arguments (optional).
 */
encloud_rc encloud_create (int argc, char *argv[], encloud_t **pencloud)
{
    encloud_rc rc = ENCLOUD_RC_SUCCESS;

    encloud_t *encloud = NULL; 

    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (pencloud == NULL, ENCLOUD_RC_BADPARAMS);

    ENCLOUD_RETURN_IF ((encloud = (encloud_t *) calloc(1, sizeof(encloud_t))) == NULL, ENCLOUD_RC_NOMEM);
    ENCLOUD_ERR_RC_IF ((encloud->context = new encloud::Context()) == NULL, ENCLOUD_RC_NOMEM);
    ENCLOUD_ERR_RC_IF (encloud->context->init(argc, argv), ENCLOUD_RC_NOMEM);

    *pencloud = encloud;

    return ENCLOUD_RC_SUCCESS;
err:
    if (encloud)
        encloud_destroy(encloud);

    return (rc ? rc : ENCLOUD_RC_GENERIC);
}

/* \brief Dispose of 'encloud' object */
encloud_rc encloud_destroy (encloud_t *encloud)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);

    ENCLOUD_DELETE(encloud->context);
    ENCLOUD_FREE(encloud);

    return ENCLOUD_RC_SUCCESS;
}

encloud_rc encloud_set_state_cb (encloud_t *encloud, encloud_state_cb state_cb, void *arg)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);
    ENCLOUD_RETURN_IF (state_cb == NULL, ENCLOUD_RC_BADPARAMS);

    return encloud->context->setStateCb(state_cb, arg);
}

encloud_rc encloud_start (encloud_t *encloud)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);

    return encloud->context->start();
}

encloud_rc encloud_stop (encloud_t *encloud)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);

    return encloud->context->stop();
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
 * Note: 'pinfo' points to an internal object (no memory management nencloudssary).
 */
ENCLOUD_DLLSPEC encloud_rc encloud_retr_info (encloud_t *encloud, encloud_info_t **pinfo)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);
    ENCLOUD_RETURN_IF (pinfo == NULL, ENCLOUD_RC_BADPARAMS);

    encloud_rc rc = ENCLOUD_RC_GENERIC;
    encloud::MessageRetrInfo msg;
    QString csrfn = encloud->cfg->config.csrTmplPath.absoluteFilePath();
    QFile csrf(csrfn);
    QString cafn = encloud->cfg->config.sslOp.caPath.absoluteFilePath();
    QFile caf(cafn);

    bool ok;

#ifdef ENCLOUD_TYPE_SECE
    msg.license = QUuid(encloud->cfg->settings->value("lic").toString());
    ENCLOUD_RETURN_IF (msg.license.isNull(), ENCLOUD_RC_NOLICENSE);

    msg.hwInfo = encloud::utils::getHwInfo();
#endif

    ENCLOUD_ERR_RC_IF ((rc = encloud->client->run(encloud::ProtocolTypeInit, msg)), rc);

    encloud->info.license_valid = msg.valid;
    encloud->info.license_expiry = msg.expiry.toTime_t();

    // save the CSR template to file
    ENCLOUD_ERR_RC_IF (!csrf.open(QIODevice::WriteOnly), ENCLOUD_RC_SYSERR);
    ENCLOUD_ERR_RC_IF (csrf.write(encloud::json::serialize(msg.csrTmpl, ok).toAscii()) == -1, ENCLOUD_RC_SYSERR);
    csrf.close();

    // save the Operation CA certificate to file
    ENCLOUD_ERR_RC_IF (!caf.open(QIODevice::WriteOnly), ENCLOUD_RC_SYSERR);
    ENCLOUD_ERR_RC_IF (caf.write(msg.caCert.toPem()) == -1, ENCLOUD_RC_SYSERR);
    caf.close();

    *pinfo = &encloud->info;

    rc = ENCLOUD_RC_SUCCESS;
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
ENCLOUD_DLLSPEC encloud_rc encloud_retr_sb_cert (encloud_t *encloud)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);

    encloud_rc rc = ENCLOUD_RC_GENERIC;
    encloud::MessageRetrCert msg;
    char *buf = NULL;
    long len;
    QString keyfn = encloud->cfg->config.sslOp.keyPath.absoluteFilePath();
    QString tmpkeyfn = keyfn + ".tmp";
    QFile tmpkey(tmpkeyfn);
    QString certfn = encloud->cfg->config.sslOp.certPath.absoluteFilePath();
    QFile certfile(certfn);

#ifdef ENCLOUD_TYPE_SECE
    msg.license = QUuid(encloud->cfg->settings->value("lic").toString());
    ENCLOUD_RETURN_IF (msg.license.isNull(), ENCLOUD_RC_NOLICENSE);

    msg.hwInfo = encloud::utils::getHwInfo();
#endif

    // generate temporary key and CSR
    ENCLOUD_ERR_IF (encloud_crypto_genkey(&encloud->crypto, encloud->cfg->config.rsaBits, qPrintable(tmpkeyfn)));
    ENCLOUD_ERR_IF (encloud_crypto_gencsr(&encloud->crypto, qPrintable(tmpkeyfn), &buf, &len));

    msg.csr = QByteArray(buf, len);

    ENCLOUD_ERR_RC_IF ((rc = encloud->client->run(encloud::ProtocolTypeInit, msg)), rc);

    // save the rencloudived certificate
    ENCLOUD_ERR_RC_IF (!certfile.open(QIODevice::WriteOnly), ENCLOUD_RC_SYSERR); 
    ENCLOUD_ERR_RC_IF (certfile.write(msg.cert.toPem()) == -1, ENCLOUD_RC_SYSERR);
    certfile.close();

    // all ok - now we can commit the temporary key
    QFile::remove(keyfn);
    ENCLOUD_RETURN_IF (!tmpkey.setPermissions(QFile::ReadOwner|QFile::WriteOwner), ENCLOUD_RC_SYSERR);
    ENCLOUD_ERR_RC_IF (!tmpkey.rename(keyfn), ENCLOUD_RC_SYSERR);

    rc = ENCLOUD_RC_SUCCESS;
err:
    if (buf)
        free(buf);
    return rc;
}

/**
 *  \brief Synchronous retrieval of configuration from SB
 *
 * The request is performed upon the Operation secure channel set up using the
 * {op_key, op_cert} pair installed via encloud_retr_sb_cert().
 *
 * Upon success, configuration info is returned to the user in 'pconf'.
 * 
 * Note: 'pconf' points to an internal object (no memory management nencloudssary).
 */
ENCLOUD_DLLSPEC encloud_rc encloud_retr_conf (encloud_t *encloud, encloud_vpn_conf_t **pconf)
{
    encloud_rc rc = ENCLOUD_RC_SUCCESS;
    encloud::MessageRetrConf msg;

    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);
    ENCLOUD_RETURN_IF (pconf == NULL, ENCLOUD_RC_BADPARAMS);

    ENCLOUD_RETURN_IF ((rc = encloud->client->run(encloud::ProtocolTypeOp, msg)), rc);

    strcpy(encloud->conf.vpn_ip, qPrintable(msg.vpnIp));
    encloud->conf.vpn_port = msg.vpnPort;
    strcpy(encloud->conf.vpn_proto, qPrintable(msg.vpnProto));
    strcpy(encloud->conf.vpn_type, qPrintable(msg.vpnType));

    *pconf = &encloud->conf;

    return rc;
}

/** \brief Is license valid? */
ENCLOUD_DLLSPEC bool encloud_info_get_license_valid (encloud_info_t *info)
{
    ENCLOUD_RETURN_IF (info == NULL, false);

    return info->license_valid;
}

/** \brief Get certificate expiry - expressed in seconds since Epoch */
ENCLOUD_DLLSPEC time_t encloud_info_get_license_expiry (encloud_info_t *info)
{
    ENCLOUD_RETURN_IF (info == NULL, -1);

    return info->license_expiry;
}
#endif

/** \brief Return ENCLOUD version string */
ENCLOUD_DLLSPEC const char *encloud_version ()
{
    return ENCLOUD_VERSION;
}

/** \brief Return ENCLOUD revision (GIT commit ID) */
ENCLOUD_DLLSPEC const char *encloud_revision ()
{
    return ENCLOUD_REVISION;
}

/** \brief Convert a return code to string representation */
ENCLOUD_DLLSPEC const char *encloud_strerror (encloud_rc rc)
{
    switch (rc)
    {
        case ENCLOUD_RC_SUCCESS:
            return "Success";
        case ENCLOUD_RC_BADPARAMS:
            return "Bad parameters";
        case ENCLOUD_RC_NOMEM:
            return "Out of memory";
        case ENCLOUD_RC_BADCONFIG:
            return "Bad configuration";
        case ENCLOUD_RC_NOLICENSE:
            return "No license found";
        case ENCLOUD_RC_CONNECT:
            return "Connection error";
        case ENCLOUD_RC_BADAUTH:
            return "Authentication failed";
        case ENCLOUD_RC_TIMEOUT:
            return "Operation timed out";
        case ENCLOUD_RC_BADRESPONSE:
            return "Bad response from peer";
        case ENCLOUD_RC_INVALIDCERT:
            return "Certificate validity check failed";
        case ENCLOUD_RC_FAILED:
            return "Operation failed";
        case ENCLOUD_RC_SYSERR:
            return "System error";
        case ENCLOUD_RC_GENERIC:
            return "Generic error";
    }

    return "<undefined>";
}

