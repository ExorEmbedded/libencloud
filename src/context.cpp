#include <openssl/x509v3.h>
#include <encloud/core.h>
#include <encloud/setup.h>
#include <encloud/vpn.h>
#include "common.h"
#include "config.h"
#include "utils.h"
#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Subject name settings from JSON CSR template */
int __encloud_context_name_cb (X509_NAME *n, void *arg);

#ifdef __cplusplus
}
#endif

encloud::Config *g_cfg = NULL;

namespace encloud {

//
// public methods
//

Context::Context ()
    : inited(false)
    , app(NULL)
    , serial(NULL)
    , poi(NULL)
{
    ENCLOUD_TRACE;
}

Context::~Context ()
{
    ENCLOUD_TRACE;

    term();
}

encloud_rc Context::init (int argc, char *argv[])
{
    ENCLOUD_TRACE;

    encloud_rc rc = ENCLOUD_RC_SUCCESS;

    ENCLOUD_RETURN_IF (this->inited, rc);

    // first off load configuration for logging
    ENCLOUD_ERR_RC_IF (this->cfg.loadFromFile(ENCLOUD_CONF_PATH), ENCLOUD_RC_BADCONFIG);

    g_cfg = &this->cfg;
    ENCLOUD_DBG(g_cfg->dump());

    if (QCoreApplication::instance() == NULL) {
        ENCLOUD_DBG ("creating internal application instance");
        ENCLOUD_ERR_RC_IF ((this->app = new QCoreApplication(argc, argv)) == NULL, ENCLOUD_RC_NOMEM);
    } else {
        ENCLOUD_DBG ("using existing application instance");
    }

#ifndef ENCLOUD_TYPE_SECE
    this->serial = utils::ustrdup(utils::file2Data(g_cfg->config.serialPath));
    ENCLOUD_ERR_RC_IF ((this->serial == NULL), ENCLOUD_RC_BADCONFIG);
    ENCLOUD_DBG("serial=" << QString::fromLocal8Bit(this->serial));

    this->poi = utils::ustrdup(utils::file2Data(g_cfg->config.poiPath));
    ENCLOUD_ERR_RC_IF ((this->poi == NULL), ENCLOUD_RC_BADCONFIG);
    ENCLOUD_DBG("poi=" << QString::fromLocal8Bit(this->poi));
#endif

    ENCLOUD_ERR_IF (this->client.setConfig(&this->cfg));

    ENCLOUD_ERR_IF (encloud_crypto_init(&this->crypto));
    ENCLOUD_ERR_IF (encloud_crypto_set_name_cb(&this->crypto, &__encloud_context_name_cb, this));

    // setup worker thread (always running but starts in stopped state)
    QObject::connect(&this->timer, SIGNAL(timeout()), &this->worker, SLOT(onTimeout()));
    QObject::connect(&this->worker, SIGNAL(finished()), &this->thread, SLOT(quit()));
    QObject::connect(this, SIGNAL(started()), &this->worker, SLOT(started()));
    QObject::connect(this, SIGNAL(stopped()), &this->worker, SLOT(stopped()));

    // connect started begin..
    this->timer.start(1000);
    this->timer.moveToThread(&this->thread);
    this->worker.moveToThread(&this->thread);
    this->thread.start();

    this->inited = true;

    return ENCLOUD_RC_SUCCESS;
err:
    term();
    return (rc ? rc : ENCLOUD_RC_GENERIC);
}

void Context::term ()
{
    ENCLOUD_TRACE;

    g_cfg = NULL;

    emit aborted();

    this->thread.wait();

    if (this->app) {
        this->app->quit();
        ENCLOUD_DELETE(this->app);
    }

#ifndef ENCLOUD_TYPE_SECE 
    ENCLOUD_FREE(this->serial);
    ENCLOUD_FREE(this->poi);
#endif
}

encloud_rc Context::setStateCb (encloud_state_cb stateCb, void *arg)
{
    return this->worker.setStateCb(stateCb, arg);
}

encloud_rc Context::start ()
{
    ENCLOUD_TRACE;

    emit started();
}

encloud_rc Context::stop ()
{
    ENCLOUD_TRACE;

    emit stopped();
}

const char *Context::getSerial () const
{
    return this->serial;
}

const char *Context::getPoi () const
{
    return this->poi;
}

const Config *Context::getConfig () const
{
    return &this->cfg;
}

#ifdef __cplusplus
extern "C" {
#endif

/* Subject name settings from JSON CSR template */
int __encloud_context_name_cb (X509_NAME *n, void *arg)
{
    ENCLOUD_RETURN_IF (n == NULL, ~0);
    ENCLOUD_RETURN_IF (arg == NULL, ~0);

    QVariant json;
    QVariantMap map;
    encloud::Context *context = (encloud::Context *) arg;
    bool ok;

    json = encloud::json::parseFromFile(context->getConfig()->config.csrTmplPath.absoluteFilePath(), ok);
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
                (const unsigned char *) context->getSerial(), -1, -1, 0));
#endif
    }

    return 0;
err:
    return ~0;
};

#ifdef __cplusplus
}
#endif

} // namespace encloud
