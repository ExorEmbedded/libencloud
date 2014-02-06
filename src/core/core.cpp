#include <openssl/x509v3.h>
#include <encloud/Core>
#include <common/config.h>
#include <common/crypto.h>
#include <setup/4ic/setup.h>
#include <setup/ece/setup.h>
#include <cloud/cloud.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

/* Subject name settings from JSON CSR template */
static int _libencloud_context_name_cb (X509_NAME *n, void *arg);

//#ifdef __cplusplus
//}
//#endif

namespace libencloud {

//
// public methods
//

Core::Core ()
    : _cfg(NULL)
    , _setup(NULL)
    , _cloud(NULL)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_initConfig());
    LIBENCLOUD_ERR_IF (_initCrypto());

#ifndef LIBENCLOUD_DISABLE_SETUP
    LIBENCLOUD_ERR_IF (_initSetup());
#endif

#ifndef LIBENCLOUD_DISABLE_CLOUD
    LIBENCLOUD_ERR_IF (_initCloud());
#endif

err:
    return;
}

Core::~Core ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE(_cfg);
    LIBENCLOUD_DELETE(_setup);
}

int Core::_initConfig ()
{
    LIBENCLOUD_TRACE;

    _cfg = new Config();
    LIBENCLOUD_ERR_IF (_cfg == NULL);

    LIBENCLOUD_ERR_IF (_cfg->loadFromFile());

    g_cfg = _cfg;
    LIBENCLOUD_DBG(g_cfg->dump());

    return 0;
err:
    return ~0;
}

int Core::_initCrypto ()
{
    libencloud_crypto_init(&_cfg->crypto);
    libencloud_crypto_set_name_cb(&_cfg->crypto, &_libencloud_context_name_cb, this);

    return 0;
}

int Core::_initSetup ()
{
    LIBENCLOUD_TRACE;

#if defined(LIBENCLOUD_MODE_4IC)
    _setup = new Q4icSetup();
#elif defined(LIBENCLOUD_MODE_ECE) || defined(LIBENCLOUD_MODE_SECE)
    _setup = new EceSetup();
#endif
    LIBENCLOUD_ERR_IF (_setup == NULL);
    _setup->setConfig(_cfg);
    _setup->init();

    connect(dynamic_cast<QObject*>(_setup), SIGNAL(stateChanged(QString)),
            this, SIGNAL(stateChanged(QString)));

    return 0;
err:
    return ~0;
}

int Core::_initCloud ()
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

    LIBENCLOUD_ERR_IF (_setup->start());

    return 0;
err:
    return ~0;
}

int Core::stop ()
{
    LIBENCLOUD_TRACE;

    return 0;
}

Config *Core::getConfig () const
{
    return _cfg;
}

} // namespace libencloud

/* Subject name settings from JSON CSR template */
static int _libencloud_context_name_cb (X509_NAME *n, void *arg)
{
    LIBENCLOUD_RETURN_IF (n == NULL, ~0);
    LIBENCLOUD_RETURN_IF (arg == NULL, ~0);

    QVariant json;
    QVariantMap map;
    libencloud::Core *core = (libencloud::Core *) arg;
    bool ok;

    json = libencloud::json::parseFromFile(core->getConfig()->config.csrTmplPath.absoluteFilePath(), ok);
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
        // now CN for ECE should be part of template and will use (unique) label given by user
#if 0
        // LIBENCLOUD: CN based on serial
        LIBENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_ASC, \
                (const unsigned char *) core->getSerial(), -1, -1, 0));
#endif

#endif
    }

    return 0;
err:
    return ~0;
};

//#endif
