#include <openssl/x509v3.h>
#include <QUuid>
#include <encloud/Core>
#include <encloud/Progress>
#include <encloud/HttpServer>
#include <common/config.h>
#include <common/crypto.h>
#include <setup/4ic/4icsetup.h>
#include <setup/ece/ecesetup.h>
#include <cloud/cloud.h>

/* Subject name settings from JSON CSR template */
static int _libencloud_context_name_cb (X509_NAME *n, void *arg);

namespace libencloud {

//
// public methods
//

Core::Core ()
    : _cfg(NULL)
    , _setup(NULL)
    , _cloud(NULL)
    , _setupState(&_setupSt)
    , _cloudState(&_cloudSt)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_initConfig());
    LIBENCLOUD_ERR_IF (_initCrypto());

#if !defined(LIBENCLOUD_DISABLE_SETUP)
    LIBENCLOUD_ERR_IF (_initSetup());
#endif

#if !defined(LIBENCLOUD_DISABLE_CLOUD)
    LIBENCLOUD_ERR_IF (_initCloud());
#endif

    LIBENCLOUD_ERR_IF (_initFsm());

err:
    return;
}

Core::~Core ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE(_cfg);
    LIBENCLOUD_DELETE(_setup);
    LIBENCLOUD_DELETE(_cloud);
}

int Core::start ()
{
    LIBENCLOUD_TRACE;

    _fsm.start();

    return 0;
}

int Core::stop ()
{
    LIBENCLOUD_TRACE;

    _fsm.stop();

    return 0;
}

Config *Core::getConfig () const
{
    return _cfg;
}

int Core::setServer (HttpServer *server)
{
    QObject *obj;

    LIBENCLOUD_ERR_IF (server == NULL);

    obj = dynamic_cast<QObject *> (server);

#ifndef LIBENCLOUD_DISABLE_CLOUD
    connect(_cloud, SIGNAL(ipAssigned(QString)), 
            obj, SLOT(vpnIpAssigned(QString)));
#endif

    obj = dynamic_cast<QObject *> (server->getHandler());

    connect(this, SIGNAL(error(QString)), 
            obj, SLOT(_coreErrorReceived(QString)));
    connect(this, SIGNAL(stateChanged(State)), 
            obj, SLOT(_coreStateChanged(State)));
    connect(this, SIGNAL(progress(Progress)), 
            obj, SLOT(_coreProgressReceived(Progress)));
    connect(this, SIGNAL(need(QString)), 
            obj, SLOT(_needReceived(QString)));

#ifdef LIBENCLOUD_MODE_SECE
    connect(obj, SIGNAL(licenseSend(QUuid)), _setupObj, SIGNAL(licenseForward(QUuid)));
#endif

    return 0;
err:
    return ~0;
}

//
// private slots
// 

void Core::_stopped ()
{
    LIBENCLOUD_TRACE;

#ifndef LIBENCLOUD_DISABLE_CLOUD
    _cloud->stop();
#endif

#ifndef LIBENCLOUD_DISABLE_SETUP
    _setup->stop();
#endif
}

void Core::_stateEntered ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state << " (" << _stateStr(state) << ")");

	if (state == _setupState)
		_setup->start();
	else if (state == _cloudState)
		_cloud->start();
}

void Core::_stateExited ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state << " (" << _stateStr(state) << ")");
}

void Core::_setupCompleted ()
{
    LIBENCLOUD_TRACE;

    const VpnConfig *vpnConfig = _setup->getVpnConfig();

    // write retrieved configuration to file
    LIBENCLOUD_ERR_IF (vpnConfig->toFile(_cfg->config.vpnConfPath.absoluteFilePath()));

err:
    return;
}

// Forward error state and message to http handler
void Core::_errorReceived (const QString &msg)
{
    emit stateChanged(StateError);
    emit error(msg);
}

// Remap step/nsteps and forward signal for http handler
void Core::_progressReceived (const Progress &p)
{
    Progress pt(p);

    // only need remapping for cloud if both setup and cloud are enabled
#if !defined(LIBENCLOUD_DISABLE_SETUP) && !defined(LIBENCLOUD_DISABLE_SETUP)
    if (sender() == _cloud)
    {
        int totalSetup = _setup->getTotalSteps();
        pt.setStep(p.getStep() + totalSetup);
        pt.setTotal(p.getStep() + totalSetup);
    }
#endif

    LIBENCLOUD_DBG("descr: " << pt.getDesc() <<
                   " step: " << pt.getStep() <<
                   " total: " << pt.getTotal());

    emit progress(pt);
}

//
// private methods
// 

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
    _setup = new Q4icSetup(_cfg);
#elif defined(LIBENCLOUD_MODE_ECE) || defined(LIBENCLOUD_MODE_SECE)
    _setup = new EceSetup(_cfg);
#endif
    LIBENCLOUD_ERR_IF (_setup == NULL);

    _setupObj = dynamic_cast<QObject *>(_setup);
    LIBENCLOUD_ERR_IF (_setupObj == NULL);

    // error signal handling
    connect(_setupObj, SIGNAL(error(QString)), this, SLOT(_errorReceived(QString)));

    // progress signal handling
    connect(_setupObj, SIGNAL(progress(Progress)), this, SLOT(_progressReceived(Progress)));

    // need message signal forwarding
    connect(_setupObj, SIGNAL(need(QString)), this, SIGNAL(need(QString)));

    // setup completion handling
    connect(_setupObj, SIGNAL(completed()), this, SLOT(_setupCompleted()));

    return 0;
err:
    return ~0;
}

int Core::_initCloud ()
{
    LIBENCLOUD_TRACE;

    _cloud = new Cloud(_cfg);
    LIBENCLOUD_ERR_IF (_cloud == NULL);

    // error signal handling
    connect(_cloud, SIGNAL(error(QString)), this, SLOT(_errorReceived(QString)));

    // progress signal handling
    connect(_cloud, SIGNAL(progress(Progress)), this, SLOT(_progressReceived(Progress)));

    // authentication request forwarding
    connect(_cloud, SIGNAL(authRequest()), this, SIGNAL(authRequest()));
    connect(_cloud, SIGNAL(proxyAuthRequest()), this, SIGNAL(proxyAuthRequest()));

    return 0;
err:
    return ~0;
}

int Core::_initFsm ()
{
    LIBENCLOUD_TRACE;
    
#if !defined(LIBENCLOUD_DISABLE_SETUP)
    LIBENCLOUD_DBG("setupState: " << _setupState);
    _initialState = _setupState;
    connect(_setupState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_setupState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _fsm.addState(_setupState);
#endif

#if !defined(LIBENCLOUD_DISABLE_CLOUD)
    LIBENCLOUD_DBG("cloudState: " << _cloudState);
#if defined(LIBENCLOUD_DISABLE_SETUP)
    _initialState = _cloudState;
#endif
    connect(_cloudState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_cloudState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _fsm.addState(_cloudState);
#endif

#if !defined(LIBENCLOUD_DISABLE_SETUP) && !defined(LIBENCLOUD_DISABLE_CLOUD)
    _setupState->addTransition(_setupObj, SIGNAL(completed()), _cloudState);
#endif

    _fsm.setInitialState(_initialState);

    connect(&_fsm, SIGNAL(stopped()), this, SLOT(_stopped()));

    return 0;
}

QString Core::_stateStr (QState *state)
{
    if (state == _setupState)
        return tr("Running Setup Module");
    else if (state == _cloudState)
        return tr("Running Cloud Enabler");
    else
        return "";
}

} // namespace libencloud

//
// static methods
// 

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
#if defined(LIBENCLOUD_MODE_SECE)
        // SECE: CN based on hw_info
        LIBENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_UTF8, \
                (const unsigned char *) libencloud::utils::getHwInfo().toUtf8().data(), -1, -1, 0));
#else
        // now CN for ECE should be part of template and will use (unique) label given by user!
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
