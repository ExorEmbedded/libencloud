#include <QtCore/QCoreApplication>
#include <QUuid>
#include <ece.h>
#include "client.h"
#include "utils.h"
#include "crypto.h"

struct ece_sb_info_s
{
    bool license_valid;
    time_t license_expiry;
};

struct ece_s
{
    QCoreApplication *app;

    Ece::Client *client;
    Ece::Config *cfg;

    QUuid license;

    ece_sb_info_t sb_info;
};

ece_rc_t ece_create (int argc, char *argv[], ece_t **ece)
{
    ece_rc_t rc = ECE_RC_SUCCESS;
    ece_t *e = NULL; 

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    ECE_RETURN_IF ((e = (ece_t *) calloc(1, sizeof(ece_t))) == NULL, ECE_RC_NOMEM);

    // only create internal QCoreApplication if there's not already one running
    if (QCoreApplication::instance() == NULL) {
        ECE_DBG ("creating internal application instance");
        ECE_ERR_RC_IF ((e->app = new QCoreApplication(argc, argv)) == NULL, ECE_RC_NOMEM);
    }

    ECE_ERR_RC_IF ((e->cfg = new Ece::Config) == NULL, ECE_RC_NOMEM);
    ECE_ERR_IF (e->cfg->loadFromFile(ECE_CONF_PATH));

    ECE_ERR_RC_IF ((e->client = new Ece::Client) == NULL, ECE_RC_NOMEM);
    ECE_ERR_IF (e->client->setConfig(e->cfg));

    *ece = e;

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

/** 
 * \brief Set license to from null-terminated 'guid'. 
 * 
 * http://harmattan-dev.nokia.com/docs/library/html/qt4/quuid.html
 */
ece_rc_t ece_set_license (ece_t *ece, const char *guid)
{
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);
    ECE_RETURN_IF (guid == NULL, ECE_RC_BADPARAMS);

    QString s(guid);
    QUuid u(s);

    ECE_ERR_IF (u.isNull());

    ECE_DBG("uiid=" << u << " variant=" << u.variant() << " version=" << u.version());

    ece->license = u;

    return ECE_RC_SUCCESS;
err:
    return ECE_RC_GENERIC;
}

ece_rc_t ece_retr_sb_info (ece_t *ece, ece_sb_info_t **info)
{
    ece_rc_t rc = ECE_RC_SUCCESS;
    Ece::MessageRetrInfo msg;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    msg.license = ece->license;
    msg.hwInfo = EceUtils::getHwInfo();

    rc = ece->client->run(Ece::ProtocolTypeInit, msg);

    ece->sb_info.license_valid = msg.valid;
    ece->sb_info.license_expiry = msg.expiry.toTime_t();

    *info = &ece->sb_info;

    return rc;
}

ece_rc_t ece_retr_sb_cert (ece_t *ece)
{
    ece_rc_t rc = ECE_RC_GENERIC;
    Ece::MessageRetrCert msg;
    //BIO *bio = NULL;
    char *buf = NULL;
    long len;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    msg.license = ece->license;
    msg.hwInfo = EceUtils::getHwInfo();

    // TODO move key file only at end
    ECE_ERR_IF (ece_crypto_genkey(ece, ece->cfg->config.rsa_bits, qPrintable(ece->cfg->config.sslOp.keyPath.absoluteFilePath())));
    ECE_ERR_IF (ece_crypto_gencsr(ece, qPrintable(ece->cfg->config.sslOp.keyPath.absoluteFilePath()), &buf, &len));

    msg.csr = QByteArray(buf, len);

    ECE_ERR_IF ((rc = ece->client->run(Ece::ProtocolTypeInit, msg)));

    rc = ECE_RC_SUCCESS;
err:
    if (buf)
        free(buf);
    return rc;
}

ece_rc_t ece_retr_sb_conf (ece_t *ece)
{
    ece_rc_t rc = ECE_RC_SUCCESS;
    Ece::MessageRetrConf msg;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    ECE_RETURN_IF ((rc = ece->client->run(Ece::ProtocolTypeOp, msg)), rc);

    return rc;
}

bool ece_sb_info_get_license_valid (ece_sb_info_t *info)
{
    return info->license_valid;
}

time_t ece_sb_info_get_license_expiry (ece_sb_info_t *info)
{
    return info->license_expiry;
}

const char *ece_version ()
{
    return ECE_VERSION;
}

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
        case ECE_RC_FAILED:
            return "Operation failed";
        case ECE_RC_GENERIC:
            return "Generic error";
    }

    return "<undefined>";
}
