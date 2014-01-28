#include <openssl/x509v3.h>
#include <QtCore/QCoreApplication>
#include <QUuid>
#include <encloud.h>
#include "client.h"
#include "utils.h"
#include "crypto.h"
#include "config.h"

encloud::Config *g_cfg = NULL;

/** \brief SB info object */
struct encloud_sb_info_s
{
    bool license_valid;
    time_t license_expiry;
};

/** \brief SB configuration object */
struct encloud_sb_conf_s
{
    char vpn_ip[ENCLOUD_DESC_SZ];
    int vpn_port;
    char vpn_proto[ENCLOUD_DESC_SZ];
    char vpn_type[ENCLOUD_DESC_SZ];
};

/** \brief ENCLOUD internal object */
struct encloud_s
{
    QCoreApplication *app;

#ifndef ENCLOUD_TYPE_SECE
    char *serial;
    char *poi;
#endif

    encloud::Client *client;
    encloud::Config *cfg;

    encloud_crypto_t crypto;
    encloud_sb_info_t sb_info;
    encloud_sb_conf_t sb_conf;
};

static int __name_cb (X509_NAME *n, void *arg);

/** 
 * \brief Create a new ENCLOUD object
 *
 * The object is returned in 'pencloud' and must be destroyed by caller using encloud_destroy().
 *
 * If no running instance of QCoreApplication is detected (libencloud used as a standalone library),
 * an internal application is created passing {argc, argv} command-line arguments (optional).
 */
encloud_rc_t encloud_create (int argc, char *argv[], encloud_t **pencloud)
{
    encloud_rc_t rc = ENCLOUD_RC_SUCCESS;
    encloud_t *e = NULL; 

    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (pencloud == NULL, ENCLOUD_RC_BADPARAMS);

    ENCLOUD_RETURN_IF ((e = (encloud_t *) calloc(1, sizeof(encloud_t))) == NULL, ENCLOUD_RC_NOMEM);
    if (QCoreApplication::instance() == NULL) {
        ENCLOUD_DBG ("creating internal application instance");
        ENCLOUD_ERR_RC_IF ((e->app = new QCoreApplication(argc, argv)) == NULL, ENCLOUD_RC_NOMEM);
    }

    ENCLOUD_ERR_RC_IF ((e->cfg = new encloud::Config) == NULL, ENCLOUD_RC_NOMEM);
    ENCLOUD_ERR_RC_IF (e->cfg->loadFromFile(ENCLOUD_CONF_PATH), ENCLOUD_RC_BADCONFIG);

    g_cfg = e->cfg;
    ENCLOUD_DBG(g_cfg->dump());

#ifndef ENCLOUD_TYPE_SECE
    e->serial = encloud::utils::ustrdup(encloud::utils::file2Data(g_cfg->config.serialPath));
    ENCLOUD_ERR_RC_IF ((e->serial == NULL), ENCLOUD_RC_BADCONFIG);
    ENCLOUD_DBG("serial=" << QString::fromLocal8Bit(e->serial));

    e->poi = encloud::utils::ustrdup(encloud::utils::file2Data(g_cfg->config.poiPath));
    ENCLOUD_ERR_RC_IF ((e->poi == NULL), ENCLOUD_RC_BADCONFIG);
    ENCLOUD_DBG("poi=" << QString::fromLocal8Bit(e->poi));
#endif

    ENCLOUD_ERR_RC_IF ((e->client = new encloud::Client) == NULL, ENCLOUD_RC_NOMEM);
    ENCLOUD_ERR_IF (e->client->setConfig(e->cfg));

    ENCLOUD_ERR_IF (encloud_crypto_init(&e->crypto));
    ENCLOUD_ERR_IF (encloud_crypto_set_name_cb(&e->crypto, &__name_cb, e));

    *pencloud = e;

    return ENCLOUD_RC_SUCCESS;
err:
    if (e->cfg)
        delete e->cfg;
    if (e->client)
        delete e->client;
    if (e)
        free(e);

    return (rc ? rc : ENCLOUD_RC_GENERIC);
}

/* \brief Dispose of 'encloud' object */
encloud_rc_t encloud_destroy (encloud_t *encloud)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);

    if (encloud->client)
        delete encloud->client;

    if (encloud->cfg) {
        g_cfg = NULL;
        delete encloud->cfg;
    }

    if (encloud->app) {
        encloud->app->quit();
        delete encloud->app;
    }

#ifndef ENCLOUD_TYPE_SECE 
    if (encloud->serial)
        free(encloud->serial);

    if (encloud->poi)
        free(encloud->poi);
#endif

    free(encloud);

    return ENCLOUD_RC_SUCCESS;
}

/** \brief Set license from null-terminated 'guid' string (SECE only)
 * 
 * The license is saved to a persistent Setting object ("lic" key in
 * ENCLOUD_SETTINGS_ORG:ENCLOUD_SETTINGS_APP as defined in defaults.h.
 */
#ifdef ENCLOUD_TYPE_SECE
ENCLOUD_DLLSPEC encloud_rc_t encloud_set_license (encloud_t *encloud, const char *guid)
{
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);
    ENCLOUD_RETURN_IF (guid == NULL, ENCLOUD_RC_BADPARAMS);

    QString s(guid);
    QUuid u(s);

    ENCLOUD_RETURN_MSG_IF (u.isNull(), ENCLOUD_RC_BADPARAMS, "bad uuid: " << guid);

    ENCLOUD_DBG("uiid=" << u << " variant=" << u.variant() << " version=" << u.version());

    encloud->cfg->settings->setValue("lic", u.toString());
    ENCLOUD_RETURN_IF (encloud->cfg->settings->status(), ENCLOUD_RC_SYSERR);

    return ENCLOUD_RC_SUCCESS;
}
#endif

/** \brief Get device's serial (ENCLOUD only) */
#ifndef ENCLOUD_TYPE_SECE
ENCLOUD_DLLSPEC const char *encloud_get_serial (encloud_t *encloud)
{
    ENCLOUD_RETURN_IF (encloud == NULL, NULL);

    return encloud->serial;
}
#endif

/** \brief Get device's PoI (ENCLOUD only) */
#ifndef ENCLOUD_TYPE_SECE
ENCLOUD_DLLSPEC const char *encloud_get_poi (encloud_t *encloud)
{
    ENCLOUD_RETURN_IF (encloud == NULL, NULL);

    return encloud->poi;
}
#endif

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
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_info (encloud_t *encloud, encloud_sb_info_t **pinfo)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);
    ENCLOUD_RETURN_IF (pinfo == NULL, ENCLOUD_RC_BADPARAMS);

    encloud_rc_t rc = ENCLOUD_RC_GENERIC;
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

    encloud->sb_info.license_valid = msg.valid;
    encloud->sb_info.license_expiry = msg.expiry.toTime_t();

    // save the CSR template to file
    ENCLOUD_ERR_RC_IF (!csrf.open(QIODevice::WriteOnly), ENCLOUD_RC_SYSERR);
    ENCLOUD_ERR_RC_IF (csrf.write(encloud::json::serialize(msg.csrTmpl, ok).toAscii()) == -1, ENCLOUD_RC_SYSERR);
    csrf.close();

    // save the Operation CA certificate to file
    ENCLOUD_ERR_RC_IF (!caf.open(QIODevice::WriteOnly), ENCLOUD_RC_SYSERR);
    ENCLOUD_ERR_RC_IF (caf.write(msg.caCert.toPem()) == -1, ENCLOUD_RC_SYSERR);
    caf.close();

    *pinfo = &encloud->sb_info;

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
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_cert (encloud_t *encloud)
{
    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);

    encloud_rc_t rc = ENCLOUD_RC_GENERIC;
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
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_conf (encloud_t *encloud, encloud_sb_conf_t **pconf)
{
    encloud_rc_t rc = ENCLOUD_RC_SUCCESS;
    encloud::MessageRetrConf msg;

    ENCLOUD_TRACE;
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);
    ENCLOUD_RETURN_IF (pconf == NULL, ENCLOUD_RC_BADPARAMS);

    ENCLOUD_RETURN_IF ((rc = encloud->client->run(encloud::ProtocolTypeOp, msg)), rc);

    strcpy(encloud->sb_conf.vpn_ip, qPrintable(msg.vpnIp));
    encloud->sb_conf.vpn_port = msg.vpnPort;
    strcpy(encloud->sb_conf.vpn_proto, qPrintable(msg.vpnProto));
    strcpy(encloud->sb_conf.vpn_type, qPrintable(msg.vpnType));

    *pconf = &encloud->sb_conf;

    return rc;
}

/** \brief Is license valid? */
ENCLOUD_DLLSPEC bool encloud_sb_info_get_license_valid (encloud_sb_info_t *info)
{
    ENCLOUD_RETURN_IF (info == NULL, false);

    return info->license_valid;
}

/** \brief Get certificate expiry - expressed in seconds since Epoch */
ENCLOUD_DLLSPEC time_t encloud_sb_info_get_license_expiry (encloud_sb_info_t *info)
{
    ENCLOUD_RETURN_IF (info == NULL, -1);

    return info->license_expiry;
}

/** \brief Get VPN IP address */
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_ip (encloud_sb_conf_t *conf)
{
    ENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_ip;
}

/** \brief Get VPN port */
ENCLOUD_DLLSPEC int encloud_sb_conf_get_vpn_port (encloud_sb_conf_t *conf)
{
    ENCLOUD_RETURN_IF (conf == NULL, -1);

    return conf->vpn_port;
}

/** \brief Get VPN protocol */
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_proto (encloud_sb_conf_t *conf)
{
    ENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_proto;
}

/** \brief Get VPN type */
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_type (encloud_sb_conf_t *conf)
{
    ENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_type;
}

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
ENCLOUD_DLLSPEC const char *encloud_strerror (encloud_rc_t rc)
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

/* Subject name settings from JSON CSR template */
static int __name_cb (X509_NAME *n, void *arg)
{
    ENCLOUD_RETURN_IF (n == NULL, ~0);
    ENCLOUD_RETURN_IF (arg == NULL, ~0);

    QVariant json;
    QVariantMap map;
    encloud_t *encloud = (encloud_t *) arg;
    bool ok;

    json = encloud::json::parseFromFile(encloud->cfg->config.csrTmplPath.absoluteFilePath(), ok);
    ENCLOUD_ERR_IF (!ok);

    map = json.toMap()["DN"].toMap();

    // parse DN fields
    for(QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter)
    {
        ENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, qPrintable(iter.key()), MBSTRING_UTF8, \
                (const unsigned char *) iter.value().toString().toUtf8().data(), -1, -1, 0));
    }

    // if CN is provided in template use it, otherwise fall back to hwinfo (SECE) or serial (ENCLOUD)
    if (map["CN"].isNull())
    {
#ifdef ENCLOUD_TYPE_SECE    
        // SECE: CN based on hw_info
        ENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_UTF8, \
                (const unsigned char *) encloud::utils::getHwInfo().toUtf8().data(), -1, -1, 0));
#else
        // ENCLOUD: CN based on serial
        ENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_ASC, \
                (const unsigned char *) encloud_get_serial(encloud), -1, -1, 0));
#endif
    }

    return 0;
err:
    return ~0;
}
