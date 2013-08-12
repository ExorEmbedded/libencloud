#include <QtCore/QCoreApplication>
#include <ece.h>
#include "client.h"

struct ece_s
{
    QCoreApplication *app;
    Ece::Client *client;
    Ece::Config *config;
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

    ECE_ERR_RC_IF ((e->config = new Ece::Config) == NULL, ECE_RC_NOMEM);
    ECE_ERR_IF (e->config->loadFromFile(ECE_CONF_PATH));

    ECE_ERR_RC_IF ((e->client = new Ece::Client) == NULL, ECE_RC_NOMEM);
    ECE_ERR_IF (e->client->setConfig(e->config));

    *ece = e;

    return ECE_RC_SUCCESS;
err:
    if (e->config)
        delete e->config;
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

    if (ece->config)
        delete ece->config;

    if (ece->app) {
        ece->app->quit();
        delete ece->app;
    }

    free(ece);

    return ECE_RC_SUCCESS;
}


ece_rc_t ece_retr_sb_info (ece_t *ece)
{
    ece_rc_t rc = ECE_RC_SUCCESS;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    rc = ece->client->run(Ece::ProtocolInitialization, Ece::MessageRetrInfo);

    return rc;
}

ece_rc_t ece_retr_sb_cert (ece_t *ece)
{
    ece_rc_t rc = ECE_RC_SUCCESS;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    rc = ece->client->run(Ece::ProtocolInitialization, Ece::MessageRetrCert);

    return rc;
}

ece_rc_t ece_retr_sb_conf (ece_t *ece)
{
    ece_rc_t rc = ECE_RC_SUCCESS;

    ECE_TRACE;
    ECE_RETURN_IF (ece == NULL, ECE_RC_BADPARAMS);

    rc = ece->client->run(Ece::ProtocolOperation, Ece::MessageRetrConf);

    return rc;
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
