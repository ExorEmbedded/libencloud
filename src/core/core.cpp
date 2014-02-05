#include <encloud/Core>
#include <common/config.h>
#include <setup/4icsetup.h>
#include <setup/ecesetup.h>
#include <cloud/cloud.h>

namespace libencloud {

Core::Core ()
    : _config(NULL)
    , _setup(NULL)
    , _cloud(NULL)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (initConfig());

#ifndef LIBENCLOUD_DISABLE_SETUP
    LIBENCLOUD_ERR_IF (initSetup());
#endif

#ifndef LIBENCLOUD_DISABLE_CLOUD
    LIBENCLOUD_ERR_IF (initCloud());
#endif

err:
    return;
}

Core::~Core ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE(_config);
    LIBENCLOUD_DELETE(_setup);
}

int Core::initConfig ()
{
    LIBENCLOUD_TRACE;

    _config = new Config();
    LIBENCLOUD_ERR_IF (_config == NULL);

    LIBENCLOUD_ERR_IF (_config->loadFromFile());

    g_cfg = _config;
    LIBENCLOUD_DBG(g_cfg->dump());

    return 0;
err:
    return ~0;
}

int Core::initSetup ()
{
    LIBENCLOUD_TRACE;

#if defined(LIBENCLOUD_MODE_4IC)
    _setup = new Q4icSetup();
#elif defined(LIBENCLOUD_MODE_ECE) || defined(LIBENCLOUD_MODE_SECE)
    _setup = new EceSetup();
#endif
    LIBENCLOUD_ERR_IF (_setup == NULL);

    connect(dynamic_cast<QObject*>(_setup), SIGNAL(stateChanged(QString)),
            this, SIGNAL(stateChanged(QString)));

    return 0;
err:
    return ~0;
}

int Core::initCloud ()
{
    LIBENCLOUD_TRACE;

    _cloud = new Cloud();
    LIBENCLOUD_ERR_IF (_cloud == NULL);

    return 0;
err:
    return ~0;
}

int Core::start ()
{
    LIBENCLOUD_TRACE;

    // TODO outer fsm
    LIBENCLOUD_ERR_IF (_setup->start());

    return 0;
err:
    return ~0;
}

void Core::timeout ()
{
    LIBENCLOUD_TRACE;
}

int Core::stop ()
{
    LIBENCLOUD_TRACE;

    return 0;
}

} // namespace libencloud

#if 0
#include <openssl/x509v3.h>
#include "common.h"
#include "config.h"
#include "utils.h"
#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Subject name settings from JSON CSR template */
int __libencloud_context_name_cb (X509_NAME *n, void *arg);

#ifdef __cplusplus
}
#endif

namespace libencloud {

//
// public methods
//

Context::Context ()
    : inited(false)
    , app(NULL)
    , serial(NULL)
    , poi(NULL)
{
    LIBENCLOUD_TRACE;
}

Context::~Context ()
{
    LIBENCLOUD_TRACE;

    term();
}

libencloud_rc Context::init (int argc, char *argv[])
{
    LIBENCLOUD_TRACE;

    libencloud_rc rc = LIBENCLOUD_RC_SUCCESS;

    LIBENCLOUD_RETURN_IF (this->inited, rc);

    // first off load configuration for logging
    LIBENCLOUD_ERR_RC_IF (this->cfg.loadFromFile(LIBENCLOUD_CONF_PATH), LIBENCLOUD_RC_BADCONFIG);

    g_cfg = &this->cfg;
    LIBENCLOUD_DBG(g_cfg->dump());

    if (QCoreApplication::instance() == NULL) {
        LIBENCLOUD_DBG ("creating internal application instance");
        LIBENCLOUD_ERR_RC_IF ((this->app = new QCoreApplication(argc, argv)) == NULL, LIBENCLOUD_RC_NOMEM);
    } else {
        LIBENCLOUD_DBG ("using existing application instance");
    }

#ifndef LIBENCLOUD_MODE_SECE
    this->serial = utils::ustrdup(utils::file2Data(g_cfg->config.serialPath));
    LIBENCLOUD_ERR_RC_IF ((this->serial == NULL), LIBENCLOUD_RC_BADCONFIG);
    LIBENCLOUD_DBG("serial=" << QString::fromLocal8Bit(this->serial));

    this->poi = utils::ustrdup(utils::file2Data(g_cfg->config.poiPath));
    LIBENCLOUD_ERR_RC_IF ((this->poi == NULL), LIBENCLOUD_RC_BADCONFIG);
    LIBENCLOUD_DBG("poi=" << QString::fromLocal8Bit(this->poi));
#endif

    LIBENCLOUD_ERR_IF (this->client.setConfig(&this->cfg));

    LIBENCLOUD_ERR_IF (libencloud_crypto_init(&this->crypto));
    LIBENCLOUD_ERR_IF (libencloud_crypto_set_name_cb(&this->crypto, &__libencloud_context_name_cb, this));

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

    return LIBENCLOUD_RC_SUCCESS;
err:
    term();
    return (rc ? rc : LIBENCLOUD_RC_GENERIC);
}

void Context::term ()
{
    LIBENCLOUD_TRACE;

    g_cfg = NULL;

    emit aborted();

    this->thread.wait();

    if (this->app) {
        this->app->quit();
        LIBENCLOUD_DELETE(this->app);
    }

#ifndef LIBENCLOUD_MODE_SECE 
    LIBENCLOUD_FREE(this->serial);
    LIBENCLOUD_FREE(this->poi);
#endif
}

libencloud_rc Context::setStateCb (libencloud_state_cb stateCb, void *arg)
{
    return this->worker.setStateCb(stateCb, arg);
}

libencloud_rc Context::start ()
{
    LIBENCLOUD_TRACE;

    emit started();
}

libencloud_rc Context::stop ()
{
    LIBENCLOUD_TRACE;

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
int __libencloud_context_name_cb (X509_NAME *n, void *arg)
{
    LIBENCLOUD_RETURN_IF (n == NULL, ~0);
    LIBENCLOUD_RETURN_IF (arg == NULL, ~0);

    QVariant json;
    QVariantMap map;
    libencloud::Context *context = (libencloud::Context *) arg;
    bool ok;

    json = libencloud::json::parseFromFile(context->getConfig()->config.csrTmplPath.absoluteFilePath(), ok);
    LIBENCLOUD_ERR_IF (!ok);

    map = json.toMap()["DN"].toMap();

    // parse DN fields
    for(QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter)
    {
        LIBENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, qPrintable(iter.key()), MBSTRING_UTF8, \
                (const unsigned char *) iter.value().toString().toUtf8().data(), -1, -1, 0));
    }

    // if CN is provided in template use it, otherwise fall back to hwinfo (SECE) or serial (LIBENCLOUD)
    if (map["CN"].isNull())
    {
#ifdef LIBENCLOUD_MODE_SECE    
        // SECE: CN based on hw_info
        LIBENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_UTF8, \
                (const unsigned char *) libencloud::utils::getHwInfo().toUtf8().data(), -1, -1, 0));
#else
        // LIBENCLOUD: CN based on serial
        LIBENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_ASC, \
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

} // namespace libencloud
#endif
