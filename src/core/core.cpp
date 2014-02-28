#include <openssl/x509v3.h>
#include <QUuid>
#include <encloud/Info>
#include <encloud/Core>
#include <encloud/Progress>
#include <encloud/HttpServer>
#include <common/config.h>
#include <common/crypto.h>
#include <common/utils.h>
#include <setup/qic/qicsetup.h>
#include <setup/ece/ecesetup.h>
#include <cloud/cloud.h>

/* Subject name settings from JSON CSR template */
static int _libencloud_context_name_cb (X509_NAME *n, void *arg);

namespace libencloud {

class AuthSession 
{
public:
    QString source;     // "setup" | "cloud"
    QString type;       // "sb" | "proxy"
};

//
// public methods
//

Core::Core ()
    : _isValid(false)
    , _cfg(NULL)
    , _setup(NULL)
    , _cloud(NULL)
    , _setupState(&_setupSt)
    , _cloudState(&_cloudSt)
{
    LIBENCLOUD_TRACE;

    qDebug() << "Starting " << qPrintable(info::versionInfo())
            << "rev: " << qPrintable(info::revision());

    LIBENCLOUD_ERR_IF (_initConfig());
    LIBENCLOUD_ERR_IF (_initCrypto());

#if !defined(LIBENCLOUD_DISABLE_SETUP)
    LIBENCLOUD_ERR_IF (_initSetup());
#endif

#if !defined(LIBENCLOUD_DISABLE_CLOUD)
    LIBENCLOUD_ERR_IF (_initCloud());
#endif

    LIBENCLOUD_ERR_IF (_initFsm());

    _isValid = true;

err:
    return;
}

Core::~Core ()
{
    LIBENCLOUD_TRACE;

    g_libencloudCfg = NULL;
    LIBENCLOUD_DELETE(_cfg);
    LIBENCLOUD_DELETE(_setup);
    LIBENCLOUD_DELETE(_cloud);
}

bool Core::isValid ()
{
    return _isValid;
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

#ifndef LIBENCLOUD_DISABLE_CLOUD
    _cloud->stop();
#endif

#ifndef LIBENCLOUD_DISABLE_SETUP
    _setup->stop();
#endif

    return 0;
}

Config *Core::getConfig () const
{
    return _cfg;
}

int Core::attachServer (HttpServer *server)
{
    QObject *obj;

    LIBENCLOUD_ERR_IF (server == NULL);

    //
    // server connections
    // 

    obj = dynamic_cast<QObject *> (server);

#ifndef LIBENCLOUD_DISABLE_CLOUD
    // ip assignments from cloud module to server
    connect(_cloud, SIGNAL(ipAssigned(QString)), 
            obj, SLOT(vpnIpAssigned(QString)));
#endif

    //
    // handler connections
    // 

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
    connect(obj, SIGNAL(licenseSend(QUuid)), 
            _setupObj, SIGNAL(licenseForward(QUuid)));
#endif

    // handle authentication received via handler
    connect(obj, SIGNAL(authSupplied(QUuid, Auth)), 
           this, SLOT(_authSupplied(QUuid, Auth)));

    // attach core configuration
    server->_cfg = _cfg;

    emit stateChanged(StateIdle);

    return 0;
err:
    return ~0;
}

//
// private slots
// 

void Core::_stateEntered ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state << " (" << _stateStr(state) << ")");

    if (state == _setupState)
    {
        emit stateChanged(StateSetup);
        _setup->start();
    }
    else if (state == _cloudState)
    {
        // VPN states emitted by cloud
        //emit stateChanged(StateConnect);
        _cloud->start();
    }
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
    LIBENCLOUD_EMIT_ERR_IF (vpnConfig == NULL, 
            error(tr("No Cloud configuration from Setup Module")));

    // write retrieved configuration to file
    LIBENCLOUD_EMIT_ERR_IF (vpnConfig->toFile(_cfg->config.vpnConfPath.absoluteFilePath()),
            error(tr("Failed writing configuration to file")));
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

// Create new auth session and emit need request for handler
void Core::_authRequired (const QString &type)
{
    QUuid uuid = QUuid::createUuid();
    QString id = utils::uuid2String(uuid);
    QString source;
    AuthSession *session = NULL;

    LIBENCLOUD_DBG("type: " << type);

    // FIXME causes fsm loops
#if 0
    // reuse last credentials if of same type
    if (_lastAuth.isValid() && _lastAuth.getType() == type)
    {
        _supplyAuthTo(sender(), _lastAuth);
        return;
    }
#endif

    if (sender() == _setupObj)
        source = "setup";
    else 
        source = "cloud";

    // check on same id
    LIBENCLOUD_ERR_IF (_authSessions.contains(id));

    // also check that we don't already have a pending request 
    // with given source + type
    foreach (session, _authSessions)
        if (session->source == source &&
                session->type == type)
            return;

    LIBENCLOUD_DBG ("adding session: " << id);

    session = new AuthSession();
    LIBENCLOUD_ERR_IF (session == NULL);

    session->source = source;
    session->type = type;

    _authSessions[id] = session;

    emit need(type + "_auth=" + id);

    return;
err:
    LIBENCLOUD_DELETE(session);
    return;
}

void Core::_authSupplied (const QUuid &uuid, const Auth &auth)
{
    QString id = utils::uuid2String(uuid);
    AuthSession *session;
    QObject *destination; 
    Auth newAuth(auth);

    LIBENCLOUD_DBG ("retrieving session: " << id);

    LIBENCLOUD_ERR_IF (!_authSessions.contains(id));

    session = _authSessions[id];
    LIBENCLOUD_ERR_IF (session == NULL);

    if (session->source == "setup")
        destination = _setupObj;
    else
        destination = _cloud;

    // get type from session
    newAuth.setType(session->type);

    _supplyAuthTo(destination, newAuth);

    LIBENCLOUD_DELETE(_authSessions[id]);

    LIBENCLOUD_ERR_IF (_authSessions.remove(id) == 0);

    _lastAuth = newAuth;
err:
    return;
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

    g_libencloudCfg = _cfg;
    LIBENCLOUD_DBG(g_libencloudCfg->dump());

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

#if defined(LIBENCLOUD_MODE_QIC)
    _setup = new QicSetup(_cfg);
#elif defined(LIBENCLOUD_MODE_ECE) || defined(LIBENCLOUD_MODE_SECE)
    _setup = new EceSetup(_cfg);
#endif
    LIBENCLOUD_ERR_IF (_setup == NULL);

    _setupObj = dynamic_cast<QObject *>(_setup);
    LIBENCLOUD_ERR_IF (_setupObj == NULL);

    // error signal handling
    connect(_setupObj, SIGNAL(error(QString)), 
            this, SLOT(_errorReceived(QString)));

    // progress signal handling
    connect(_setupObj, SIGNAL(progress(Progress)), 
            this, SLOT(_progressReceived(Progress)));

    // need message signal forwarding
    connect(_setupObj, SIGNAL(need(QString)), 
            this, SIGNAL(need(QString)));

    // setup completion handling
    connect(_setupObj, SIGNAL(completed()), 
            this, SLOT(_setupCompleted()));

    // authentication forwarding
    connect(_setupObj, SIGNAL(authRequired(QString)),
            this, SLOT(_authRequired(QString)));

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
    connect(_cloud, SIGNAL(error(QString)), 
            this, SLOT(_errorReceived(QString)));

    // state changes forwarding for connecting/connected states
    connect(_cloud, SIGNAL(stateChanged(State)), 
            this, SIGNAL(stateChanged(State)));

    // progress signal handling
    connect(_cloud, SIGNAL(progress(Progress)), 
            this, SLOT(_progressReceived(Progress)));

    // need message signal forwarding
    connect(_cloud, SIGNAL(need(QString)), 
            this, SIGNAL(need(QString)));

    // authentication forwarding
    connect(_cloud, SIGNAL(authRequired(QString)),
            this, SLOT(_authRequired(QString)));

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
    connect(_setupState, SIGNAL(entered()), 
            this, SLOT(_stateEntered()));
    connect(_setupState, SIGNAL(exited()), 
            this, SLOT(_stateExited()));
    _fsm.addState(_setupState);
#endif

#if !defined(LIBENCLOUD_DISABLE_CLOUD)
    LIBENCLOUD_DBG("cloudState: " << _cloudState);
#if defined(LIBENCLOUD_DISABLE_SETUP)
    _initialState = _cloudState;
#endif
    connect(_cloudState, SIGNAL(entered()), 
            this, SLOT(_stateEntered()));
    connect(_cloudState, SIGNAL(exited()), 
            this, SLOT(_stateExited()));
    _fsm.addState(_cloudState);
#endif

#if !defined(LIBENCLOUD_DISABLE_SETUP) && !defined(LIBENCLOUD_DISABLE_CLOUD)
    _setupState->addTransition(_setupObj, SIGNAL(completed()), _cloudState);
#endif

    _fsm.setInitialState(_initialState);

    return 0;
}

void Core::_supplyAuthTo (QObject *dest, const Auth &auth)
{
    // connect temporarily only to emit signal to proper destination
    connect(this, SIGNAL(authSupplied(Auth)), 
            dest, SIGNAL(authSupplied(Auth)));
    emit authSupplied(auth);
    disconnect(this, SIGNAL(authSupplied(Auth)), 
            dest, SIGNAL(authSupplied(Auth)));
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

    json = libencloud::json::parseFromFile(core->getConfig()->
            config.csrTmplPath.absoluteFilePath(), ok);
    LIBENCLOUD_ERR_IF (!ok);

    map = json.toMap()["DN"].toMap();

    // parse DN fields
    for(QVariantMap::const_iterator iter = map.begin(); 
            iter != map.end(); ++iter)
    {
        LIBENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, 
                    qPrintable(iter.key()), MBSTRING_UTF8,
                    (const unsigned char *)
                    iter.value().toString().toUtf8().data(),
                    -1, -1, 0));
    }

    // if CN is provided in template use it, otherwise fall back to hwinfo
    // (SECE) or serial (LIBENCLOUD)
    if (map["CN"].isNull())
    {
#if defined(LIBENCLOUD_MODE_SECE)
        // SECE: CN based on hw_info
        LIBENCLOUD_ERR_IF (!X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_UTF8,
                (const unsigned char *)
                libencloud::utils::getHwInfo().toUtf8().data(), 
                -1, -1, 0));
#else
        // now CN for ECE should be part of template and will use (unique)
        // label given by user!
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
}
