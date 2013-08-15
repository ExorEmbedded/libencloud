#include <openssl/x509v3.h>
#include <QtCore/QCoreApplication>
#include <QUuid>
#include <ece.h>
#include "client.h"
#include "utils.h"
#include "crypto.h"

/** \brief SB info object */
struct ece_sb_info_s
{
    bool license_valid;
    time_t license_expiry;
    QVariant csr_tmpl;
};

/** \brief SB configuration object */
struct ece_sb_conf_s
{
    char vpn_ip[ECE_DESC_SZ];
    int vpn_port;
    char vpn_proto[ECE_DESC_SZ];
    char vpn_type[ECE_DESC_SZ];
};

/** \brief ECE internal object */
struct ece_s
{
    QCoreApplication *app;
    QUuid license;

    Ece::Client *client;
    Ece::Config *cfg;

    ece_crypto_t crypto;
    ece_sb_info_t sb_info;
    ece_sb_conf_t sb_conf;
};

static int __name_cb (X509_NAME *n, void *arg);

/** 
 * \brief Create a new ECE object
 *
 * The object is returned in 'pece' and must be destroyed by caller using ece_destroy().
 *
 * If no running instance of QCoreApplication is detected (libece used as a standalone library),
 * an internal application is created passing {argc, argv} command-line arguments (optional).
 */
ece_rc_t ece_create (int argc, char *argv[], ece_t **pece)
{
    ece_rc_t rc = ECE_RC_SUCCESS;
    ece_t *e = NULL; 

    ECE_TRACE;
    ECE_RETURN_IF (pece == NULL, ECE_RC_BADPARAMS);

    ECE_RETURN_IF ((e = (ece_t *) calloc(1, sizeof(ece_t))) == NULL, ECE_RC_NOMEM);

    if (QCoreApplication::instance() == NULL) {
        ECE_DBG ("creating internal application instance");
        ECE_ERR_RC_IF ((e->app = new QCoreApplication(argc, argv)) == NULL, ECE_RC_NOMEM);
    }

    ECE_ERR_RC_IF ((e->cfg = new Ece::Config) == NULL, ECE_RC_NOMEM);
    ECE_ERR_IF (e->cfg->loadFromFile(ECE_CONF_PATH));

    ECE_ERR_RC_IF ((e->client = new Ece::Client) == NULL, ECE_RC_NOMEM);
    ECE_ERR_IF (e->client->setConfig(e->cfg));

    ECE_ERR_IF (ece_crypto_init(&e->crypto));
    ECE_ERR_IF (ece_crypto_set_name_cb(&e->crypto, &__name_cb, &e->sb_info));

    *pece = e;

    return ECE_RC_SUCCESS;
err:
    if (e->cfg)
        delete e->cfg;
    if (e->client)
        delete e->client;
    if (e)
        free(e);

    return (rc ? rc : ECE_RC_GENERIC);
}

/* \brief Dispose of 'ece' object */
ece_rc_t ece_destroy (ece_t *ece)
{
    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    if (ece->client)
        delete ece->client;

    if (ece->cfg)
        delete ece->cfg;

    if (ece->app) {
        ece->app->quit();
        delete ece->app;
    }

    free(ece);

    return ECE_RC_SUCCESS;
}

/** \brief Set license from null-terminated 'guid' string */
ece_rc_t ece_set_license (ece_t *ece, const char *guid)
{
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);
    ECE_RETURN_IF (guid == NULL, ECE_RC_BADPARAMS);

    QString s(guid);
    QUuid u(s);

    ECE_RETURN_MSG_IF (u.isNull(), ECE_RC_BADPARAMS, "bad uuid: " << guid);

    ECE_DBG("uiid=" << u << " variant=" << u.variant() << " version=" << u.version());

    ece->license = u;

    return ECE_RC_SUCCESS;
}

/**
 *  \brief Synchronous retrieval of info from SB
 *
 * The request is performed upon the Initialization secure channel set up using the
 * {key1, cert1} pair installed during production phase.
 * 
 * The license set using ece_set_license() along with retrieved hardware info are sent
 * as proof of identity.
 * 
 * License info is returned to the user and CSR template is stored internally.
 * 
 * Note: 'pinfo' points to an internal object (no memory management necessary).
 */
ece_rc_t ece_retr_sb_info (ece_t *ece, ece_sb_info_t **pinfo)
{
    ece_rc_t rc = ECE_RC_GENERIC;
    Ece::MessageRetrInfo msg;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);
    ECE_RETURN_IF (pinfo == NULL, ECE_RC_BADPARAMS);

    ECE_RETURN_IF (ece->license.isNull(), ECE_RC_BADPARAMS);

    msg.license = ece->license;
    msg.hwInfo = EceUtils::getHwInfo();

    ECE_ERR_RC_IF ((rc = ece->client->run(Ece::ProtocolTypeInit, msg)), rc);

    ece->sb_info.license_valid = msg.valid;
    ece->sb_info.license_expiry = msg.expiry.toTime_t();
    ece->sb_info.csr_tmpl = msg.csr_tmpl;

    *pinfo = &ece->sb_info;

    rc = ECE_RC_SUCCESS;
err:
    return rc;
}

/**
 *  \brief Synchronous retrieval of certificate from SB
 *
 * The request is performed upon the Initialization secure channel set up using the
 * {key1, cert1} pair installed during production phase.
 * 
 * The license set using ece_set_license() along with retrieved hardware info are sent
 * as proof of identity. An internal key is generated and used to generate a Certificate
 * Signing Request which is sent to SB. 
 * 
 * Upon success, a new certificate is returned and stored internally.
 */
ece_rc_t ece_retr_sb_cert (ece_t *ece)
{
    ece_rc_t rc = ECE_RC_GENERIC;
    Ece::MessageRetrCert msg;
    char *buf = NULL;
    long len;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    ECE_RETURN_IF (ece->license.isNull(), ECE_RC_BADPARAMS);

    QString keyfn = ece->cfg->config.sslOp.keyPath.absoluteFilePath();
    QString tmpkeyfn = keyfn + ".tmp";
    QFile tmpkey(tmpkeyfn);
    QString certfn = ece->cfg->config.sslOp.certPath.absoluteFilePath();
    QFile certfile(certfn);

    msg.license = ece->license;
    msg.hwInfo = EceUtils::getHwInfo();

    // generate temporary key and CSR
    ECE_ERR_IF (ece_crypto_genkey(&ece->crypto, ece->cfg->config.rsa_bits, qPrintable(tmpkeyfn)));
    ECE_ERR_IF (ece_crypto_gencsr(&ece->crypto, qPrintable(tmpkeyfn), &buf, &len));

    msg.csr = QByteArray(buf, len);

    ECE_ERR_RC_IF ((rc = ece->client->run(Ece::ProtocolTypeInit, msg)), rc);

    // save the received certificate
    ECE_ERR_RC_IF (!certfile.open(QIODevice::WriteOnly), ECE_RC_SYSERR); 
    ECE_ERR_RC_IF (certfile.write(msg.cert.toPem()) == -1, ECE_RC_SYSERR);
    certfile.close();

    // all ok - now we can commit the temporary key
    ECE_ERR_RC_IF (!QFile::remove(keyfn), ECE_RC_SYSERR);
    ECE_ERR_RC_IF (!tmpkey.rename(keyfn), ECE_RC_SYSERR);

    rc = ECE_RC_SUCCESS;
err:
    if (buf)
        free(buf);
    return rc;
}

/**
 *  \brief Synchronous retrieval of configuration from SB
 *
 * The request is performed upon the Operation secure channel set up using the
 * {key2, cert2} pair installed via ece_retr_sb_cert().
 *
 * Upon success, configuration info is returned to the user in 'pconf'.
 * 
 * Note: 'pconf' points to an internal object (no memory management necessary).
 */
ece_rc_t ece_retr_sb_conf (ece_t *ece, ece_sb_conf_t **pconf)
{
    ece_rc_t rc = ECE_RC_SUCCESS;
    Ece::MessageRetrConf msg;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);
    ECE_RETURN_IF (pconf == NULL, ECE_RC_BADPARAMS);

    ECE_RETURN_IF ((rc = ece->client->run(Ece::ProtocolTypeOp, msg)), rc);

    strcpy(ece->sb_conf.vpn_ip, qPrintable(msg.vpnIp));
    ece->sb_conf.vpn_port = msg.vpnPort;
    strcpy(ece->sb_conf.vpn_proto, qPrintable(msg.vpnProto));
    strcpy(ece->sb_conf.vpn_type, qPrintable(msg.vpnType));

    *pconf = &ece->sb_conf;

    return rc;
}

/** \brief Is license valid? */
bool ece_sb_info_get_license_valid (ece_sb_info_t *info)
{
    return info->license_valid;
}

/** \brief Get certificate expiry - expressed in seconds since Epoch */
time_t ece_sb_info_get_license_expiry (ece_sb_info_t *info)
{
    return info->license_expiry;
}

/** \brief Get VPN IP address */
char *ece_sb_conf_get_vpn_ip (ece_sb_conf_t *conf)
{
    return conf->vpn_ip;
}

/** \brief Get VPN port */
int ece_sb_conf_get_vpn_port (ece_sb_conf_t *conf)
{
    return conf->vpn_port;
}

/** \brief Get VPN protocol */
char *ece_sb_conf_get_vpn_proto (ece_sb_conf_t *conf)
{
    return conf->vpn_proto;
}

/** \brief Get VPN type */
char *ece_sb_conf_get_vpn_type (ece_sb_conf_t *conf)
{
    return conf->vpn_type;
}

/** \brief Return ECE version string */
const char *ece_version ()
{
    return ECE_VERSION;
}

/** \brief Convert a return code to string representation */
const char *ece_strerror (ece_rc_t rc)
{
    switch (rc)
    {
        case ECE_RC_SUCCESS:
            return "Success";
        case ECE_RC_BADPARAMS:
            return "Bad parameters";
        case ECE_RC_NOMEM:
            return "Out of memory";
        case ECE_RC_CONNECT:
            return "Connection error";
        case ECE_RC_BADAUTH:
            return "Authentication failed";
        case ECE_RC_TIMEOUT:
            return "Operation timed out";
        case ECE_RC_BADRESPONSE:
            return "Bad response from peer";
        case ECE_RC_FAILED:
            return "Operation failed";
        case ECE_RC_SYSERR:
            return "System error";
        case ECE_RC_GENERIC:
            return "Generic error";
    }

    return "<undefined>";
}

/* Subject name settings from retrieved JSON CSR template */
static int __name_cb (X509_NAME *n, void *arg)
{
    ECE_RETURN_IF (n == NULL, ~0);
    ECE_RETURN_IF (arg == NULL, ~0);

    ece_sb_info_t *sb_info = (ece_sb_info_t *) arg;

    QVariantMap map = sb_info->csr_tmpl.toMap()["DN"].toMap();

    // parse DN fields
    for(QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter)
    {
        ECE_ERR_IF (!X509_NAME_add_entry_by_txt(n, qPrintable(iter.key()), MBSTRING_UTF8, \
                (const unsigned char *) iter.value().toString().toUtf8().data(), -1, -1, 0));
    }

    // CN based on hw_info
    EceUtils::getHwInfo();
    ECE_ERR_IF (!X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_UTF8, \
            (const unsigned char *) EceUtils::getHwInfo().toUtf8().data(), -1, -1, 0));

    return 0;
err:
    return ~0;
}
