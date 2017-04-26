#include <QFileInfo>
#include <QSslCertificate>
#include <QUuid>
#include <encloud/Core>
#ifndef Q_OS_WINCE
#include <openssl/x509v3.h>
#include <encloud/Crypto>
#endif
#include <encloud/Info>
#include <encloud/Logger>
#include <encloud/Progress>
#include <encloud/Proxy>
#include <encloud/Utils>
#include <encloud/Api/CommonApi>
#include <common/config.h>
#ifdef LIBENCLOUD_MODE_QCC
#include <setup/qcc/qccsetup.h>
#include <setup/reg/regsetup.h>
#endif
#if defined(LIBENCLOUD_MODE_ECE) || defined(LIBENCLOUD_MODE_SECE)
#include <setup/ece/ecesetup.h>
#endif
#ifdef LIBENCLOUD_MODE_VPN
#include <setup/vpn/vpnsetup.h>
#endif
#include <cloud/cloud.h>


#ifndef Q_OS_WINCE
/* Subject name settings from JSON CSR template */
static int _libencloud_context_name_cb (X509_NAME *n, void *arg);
#endif

namespace libencloud {

//
// public methods
//

Core::Core (Mode mode)
    : _isValid(false)
    , _mode(mode)
    , _state(StateIdle)
    , _setup(NULL)
    , _setupObj(NULL)
    , _cloud(NULL)
    , _cloudObj(NULL)
    , _setupState(&_setupSt)
    , _cloudState(&_cloudSt)
    , _cloudApi(NULL)
    , _clientPort(-1)
    , _logPort(-1)
    , _qnam(NULL)
    , _networkManager(NULL)
    , _proxyFactory(NULL)
{
    LIBENCLOUD_TRACE;

    qDebug() << "Starting " << qPrintable(info::versionInfo())
            << " rev: " << qPrintable(info::revision())
            << " mode: " << QString::number(_mode);

    LIBENCLOUD_ERR_IF (_init());
    LIBENCLOUD_ERR_IF (_initConfig());
    LIBENCLOUD_ERR_IF (_initCrypto());

    // Setup and cloud initally use defaults
#if !defined(LIBENCLOUD_DISABLE_SETUP)
    LIBENCLOUD_ERR_IF (_initSetup());
#endif

#if !defined(LIBENCLOUD_DISABLE_CLOUD)
    LIBENCLOUD_ERR_IF (_initCloud());
#endif

    LIBENCLOUD_ERR_IF (_initApi());
    LIBENCLOUD_ERR_IF (_initFsm());

    _isValid = true;

err:
    return;
}

Core::~Core ()
{
    LIBENCLOUD_TRACE;

    g_libencloudCfg = NULL;

    LIBENCLOUD_DELETE(_qnam);
    LIBENCLOUD_DELETE(_networkManager);
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

    LIBENCLOUD_ERR_IF (_state == StateConnect || _state == StateCloud);

    // Connect Client starts immediately
    if (!_cfg->config.decongest)
    {
        _start();
        return 0;
    }

    // for ECE and SECE startup is automated so we introduce a random pause of
    // 0-5 secs to avoid server congestion due to excessive simultaneous connections
    {
        int msecs = qRound(5000 * ((qreal) qrand() / (qreal) RAND_MAX));

        LIBENCLOUD_DBG(QString("Congestion avoidance - waiting %1 ms")
                .arg(QString::number(msecs)));

        QTimer::singleShot(msecs, this, SLOT(_start()));
    }

    return 0;
err:
    return ~0;
}

void Core::_start ()
{
    _fsm.start();

    if (_clientPort != -1)
        _clientWatchdog.start();
}

int Core::stop ()
{
    LIBENCLOUD_TRACE;

    if (_clientPort != -1)
        _clientWatchdog.stop();

    _fsm.stop();
    _clientWatchdog.stop();

    // make sure routes are removed before client is stopped
    _networkManager->stop();

#ifndef LIBENCLOUD_DISABLE_SETUP
    _setup->stop(true, (_state == StateConnect || _state == StateCloud));
#endif

    // _setupStopped() will be triggered

    return 0;
}

Config *Core::getConfig () const
{
    return _cfg;
}

int Core::attachServer (Server *server)
{
    QObject *obj;
    HttpHandler *handler;

    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (server == NULL);

    //
    // server connections
    // 

    obj = server;

#ifndef LIBENCLOUD_DISABLE_CLOUD
    // ip assignments from cloud module to server
    connect(_cloud, SIGNAL(ipAssigned(QString)), 
            obj, SLOT(vpnIpAssigned(QString)));
    // from server to local objects
    connect(obj, SIGNAL(configSupply(QVariant)),
            _cfg, SLOT(receive(QVariant)));
    connect(obj, SIGNAL(configSupply(QVariant)),
            this, SLOT(_configReceived(QVariant)));
    connect(obj, SIGNAL(authSupply(libencloud::Auth)), 
            this, SLOT(_authSupplied(libencloud::Auth)));
    connect(obj, SIGNAL(actionRequest(QString, libencloud::Params)), 
            this, SLOT(_actionRequest(QString, libencloud::Params)));
#endif

#ifndef Q_OS_WINCE
    handler = dynamic_cast<HttpHandler *> (server->getHandler());
#else
    handler = (HttpHandler *) (server->getHandler());
#endif
    LIBENCLOUD_ERR_IF (handler == NULL);

    //
    // handler setup
    // 

#ifdef LIBENCLOUD_MODE_ECE
    if (_cfg->config.poiPath.exists())
    {
        LIBENCLOUD_DBG("Reading PoI from file: " <<
                _cfg->config.poiPath.absoluteFilePath());

        LIBENCLOUD_ERR_IF (handler->setPoi(QUuid(utils::file2Data(_cfg->config.poiPath))));
    }
    else if (_cfg->config.sslInit.certPath.exists())
    {
        LIBENCLOUD_DBG("Reading PoI from cert: " <<
                _cfg->config.sslInit.certPath.absoluteFilePath());

        QList<QSslCertificate> initCerts(QSslCertificate::fromPath(
                    _cfg->config.sslInit.certPath.absoluteFilePath()));
        QSslCertificate initCert = initCerts.first();
        QString poiStr;

        LIBENCLOUD_ERR_IF (initCerts.isEmpty());
        LIBENCLOUD_ERR_IF (initCert.isNull());

        poiStr = initCert.subjectInfo(QSslCertificate::CommonName);

        LIBENCLOUD_DBG("PoI: " << poiStr);
        LIBENCLOUD_ERR_IF (handler->setPoi(QUuid(poiStr)));
    }
#endif

    //
    // handler connections
    // 

    obj = handler;

    connect(this, SIGNAL(error(libencloud::Error)), 
            obj, SLOT(_coreErrorReceived(libencloud::Error)));
    connect(this, SIGNAL(stateChanged(State)), 
            obj, SLOT(_coreStateChanged(State)));
    connect(this, SIGNAL(progress(Progress)), 
            obj, SLOT(_coreProgressReceived(Progress)));
    connect(this, SIGNAL(fallback(bool)), 
            obj, SLOT(_coreFallbackReceived(bool)));
    connect(this, SIGNAL(need(QString, QVariant)), 
            obj, SLOT(_needReceived(QString, QVariant)));

#ifdef LIBENCLOUD_MODE_SECE
    connect(obj, SIGNAL(licenseSend(QUuid)), 
            _setupObj, SIGNAL(licenseForward(QUuid)));
#endif

#ifdef LIBENCLOUD_MODE_QCC
    connect(obj, SIGNAL(clientPortSend(int)), 
            this, SLOT(_clientPortReceived(int)));
    connect(obj, SIGNAL(logPortSend(int)), 
            this, SLOT(_logPortReceived(int)));
    connect(this, SIGNAL(serverConfigSupply(QVariant)), 
            obj, SLOT(_serverConfigReceived(QVariant)));
#endif

    // when auth is supplied, it is forwarded to all modules, while
    // authentication requests are reemitted in _authRequired as "need" signals
    // for handler
    connect(obj, SIGNAL(authSupplied(libencloud::Auth)), 
           this, SLOT(_authSupplied(libencloud::Auth)));
    connect(obj, SIGNAL(actionRequest(QString, libencloud::Params)), 
            this, SLOT(_actionRequest(QString, libencloud::Params)));
    connect(obj, SIGNAL(configSupplied(QVariant)),
            _cfg, SLOT(receive(QVariant)));
    connect(obj, SIGNAL(configSupplied(QVariant)),
            this, SLOT(_configReceived(QVariant)));

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

void Core::_stateChanged (State state)
{
    LIBENCLOUD_DBG("[Core] state: " << QString::number(state) << " (" <<
            stateToString(state) << ")");

    _state = state;
}

void Core::_stateEntered ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("[Core] state: " << state << " (" << _stateStr(state) << ")");

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

    LIBENCLOUD_DBG("[Core] state: " << state << " (" << _stateStr(state) << ")");
}

void Core::_setupCompleted ()
{
    LIBENCLOUD_TRACE;

    const VpnConfig *vpnConfig = _setup->getVpnConfig();
    const VpnConfig *fallbackVpnConfig = _setup->getFallbackVpnConfig();

    LIBENCLOUD_EMIT_ERR_IF (vpnConfig == NULL,
            error(Error(tr("No Cloud configuration from Setup Module"))));
    if (!vpnConfig->isFile()) {
        LIBENCLOUD_EMIT_ERR_IF (vpnConfig->toFile(
                                    _cfg->config.vpnConfPath.absoluteFilePath()),
                                error(Error(tr("Failed writing configuration to file"))));
    }

    if (fallbackVpnConfig && fallbackVpnConfig->isValid())
    {
        LIBENCLOUD_DBG("[Core] Fallback configuration found");
        if (!fallbackVpnConfig->isFile()) {
            LIBENCLOUD_EMIT_ERR_IF (fallbackVpnConfig->toFile(
                                        _cfg->config.fallbackVpnConfPath.absoluteFilePath()),
                                    error(Error(tr("Failed writing fallback configuration to file"))));
        }
    }
err:
    return;
}

void Core::_setupStopped ()
{
#ifndef LIBENCLOUD_DISABLE_CLOUD
    _cloud->stop();
#endif

    emit authSupplied(Auth());
    emit stateChanged(StateIdle);
    emit progress(Progress());
}

void Core::_fallback (bool isFallback)
{
    LIBENCLOUD_DBG("[Core] fallback: " << isFallback);

    _networkManager->setFallback(isFallback);

    emit fallback(isFallback);
}

// Forward error state and message to http handler
void Core::_errorReceived (const libencloud::Error &err)
{
    LIBENCLOUD_DBG("[Core] " << err.toString());

    switch (err.getCode())
    {
      // non-critical errors - just stop FSM
        case libencloud::Error::CodeAuthDomainRequired:
            // stop setup without resetting cached data
            _setup->stop(false);
            _fsm.stop();
            return;

       // critical errors
       default:
           // QCC stops progress upon critical errors for user intervention
           // while ECE and SECE keep on retrying automatically (in internal modules)
           if (!_cfg->config.autoretry)
               stop();
           break;
    }

    emit stateChanged(StateError);
    emit error(err);
}

void Core::_configReceived (const QVariant &config)
{
    // ignore resets
    if (config.toMap()["reset"].toBool())
        return;

    if (config.toMap()["setup"].toMap()["agent"].isNull())
        return;

    _cfg->config.setupAgent = config.toMap()["setup"].toMap()["agent"].toBool();

#if !defined(LIBENCLOUD_DISABLE_SETUP)
    LIBENCLOUD_ERR_IF (_initSetup());
#endif

#if !defined(LIBENCLOUD_DISABLE_CLOUD)
    _cloud->setSetup(_setup);
#endif

    LIBENCLOUD_ERR_IF (_initFsm());
err:
    return;
}

// Remap step/nsteps and forward signal for http handler
void Core::_progressReceived (const Progress &p)
{
    Progress pt(p);

    // only need remapping if both setup and cloud are enabled
#if !defined(LIBENCLOUD_DISABLE_SETUP) && !defined(LIBENCLOUD_DISABLE_SETUP)
    if (sender() == _cloud)
        pt.setStep(p.getStep() + _setup->getTotalSteps());

    pt.setTotal(_setup->getTotalSteps() + _cloud->getTotalSteps());
#endif

    LIBENCLOUD_DBG("[Core] descr: " << pt.getDesc() <<
                   " step: " << pt.getStep() <<
                   " total: " << pt.getTotal());

    emit progress(pt);
}

void Core::_authSupplied (const libencloud::Auth &auth)
{
    LIBENCLOUD_DBG("[Core] " << auth.toString());

    // proxy settings handled globally
    switch (auth.getId())
    {
        case Auth::SwitchboardId:
            _sbAuth = auth;

            // inclusive proxy factory logic (default) => add Switchboard host
            if (_proxyFactory)
                _proxyFactory->add(QUrl(_sbAuth.getUrl()).host());

            break;
        case Auth::ProxyId:
        {
            QUrl url(auth.getUrl());

            QNetworkProxy proxy(
                Auth::typeToQt(auth.getType()),
                url.host(),
                url.port(),
                auth.getUser(),
                auth.getPass()
                );

            _proxyFactory = new libencloud::ProxyFactory;
            LIBENCLOUD_ERR_IF (_proxyFactory == NULL);

            _proxyFactory->setApplicationProxy(auth.getType() == Auth::NoneType ? 
                    QNetworkProxy::NoProxy : proxy);
            if (_sbAuth.getUrl() != "")
                _proxyFactory->add(QUrl(_sbAuth.getUrl()).host());

            LIBENCLOUD_DBG("[Core] Setting application proxy factory");

            // lib takes ownership of our proxy factory
            QNetworkProxyFactory::setApplicationProxyFactory(_proxyFactory);  

            _proxyAuth = auth;
            break;
        }
        default:
            break;
    }

    emit authSupplied(auth);

    return;
err:
    LIBENCLOUD_DELETE(_proxyFactory);
    return;
}

void Core::_authRequired (libencloud::Auth::Id id, QVariant params)
{
    switch (id)
    {
        case Auth::SwitchboardId:
        {
            QVariantMap needParams;
            needParams["domains"] = params.toList();
            emit need("sb_auth", needParams);
            break;
        }
        case Auth::ProxyId:
            emit need("proxy_auth", QVariant());
            break;
        default:
            LIBENCLOUD_ERR_IF (1);
            break;
    }
err:
    return;
}

void Core::_serverConfigReceived (const QVariant &variant)
{
    LIBENCLOUD_TRACE;

    QVariantMap serverMap = variant.toMap();

    LIBENCLOUD_ERR_IF (serverMap["server"].isNull());
    serverMap = serverMap["server"].toMap();

    LIBENCLOUD_ERR_IF (serverMap["openvpn_internal_ip"].isNull());
    _networkManager->setGateway(serverMap["openvpn_internal_ip"].toString());

    if (!serverMap["fallback_openvpn_internal_ip"].isNull())
        _networkManager->setFallbackGateway(serverMap["fallback_openvpn_internal_ip"].toString());
err:
    return;
}

// Setup watchdog which is started/stopped according to core state
void Core::_clientPortReceived (int port)
{
    QString sp = QString::number(port);
    QUrl url;

    LIBENCLOUD_DBG ("[Core] client port: " << sp);

    url.setScheme(LIBENCLOUD_API_SCHEME);
    url.setHost(LIBENCLOUD_API_HOST);
    url.setPort(port);
    url.setPath(LIBENCLOUD_API_STATUS_PATH);

    _clientWatchdog.setUrl(url);
    // _clientDown() will be invoked upon down() signal

    _clientPort = port;
}

void Core::_logPortReceived (int port)
{
    QString sp = QString::number(port);

    LIBENCLOUD_DBG ("[Core] log port: " << sp);

    _logPort = port;

    Logger::connectToListener("tcp://127.0.0.1:" + sp);
}

// This handler is triggered for all API receivers
void Core::_actionRequest (const QString &action, const libencloud::Params &params)
{
    LIBENCLOUD_DBG ("action: " << action);
    
    //
    // handled locally
    //
    if (action == "start")
        LIBENCLOUD_ERR_IF (start());
    else if (action == "stop")
        LIBENCLOUD_ERR_IF (stop());
    //
    // handled by NetworkManager
    //
    else if (action == "syncRoutes")
        _networkManager->syncRoutes(paramsFind(params, "ips").split(","));
    //
    // forwarded to GUI service
    //
    else if (action == "open" ||
            action == "close")
    {
        switch (_mode)
        {
            case EncloudMode:
                // by now we whould have received port setting from client
                LIBENCLOUD_ERR_IF (_clientPort == -1);
                _cloudApi->setPort(_clientPort);
                break;
            case GuiMode:
                break;
        }
        emit actionRequest(action, params);
    }
    else if (action == "setGateway")
        _networkManager->setGateway(paramsFind(params, "ip"));
    else 
        emit error(Error(tr("Invalid action: ") + action));

err:
    return;
}

// If watchdog notifies a down stop the service (closed or crashed)
void Core::_clientDown ()
{
    LIBENCLOUD_TRACE;

    stop();
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
    // initialize Qt's PRNG
    qsrand((uint) QTime::currentTime().msec());

#ifndef Q_OS_WINCE
    libencloud_crypto_init(&_cfg->crypto);
    libencloud_crypto_set_name_cb(&_cfg->crypto, &_libencloud_context_name_cb, this);
#endif

    return 0;
}

int Core::_initSetup ()
{
    LIBENCLOUD_TRACE;

    // reset pre-existing instances
    LIBENCLOUD_DELETE_LATER(_setup);

#if defined(LIBENCLOUD_MODE_QCC)
    LIBENCLOUD_DBG("[Core] setupAgent: " << _cfg->config.setupAgent);
    if (_cfg->config.setupAgent)
        _setup = new RegSetup(_cfg);  // agent mode
    else
        _setup = new QccSetup(_cfg);  // app mode (default)
#elif defined(LIBENCLOUD_MODE_ECE) || defined(LIBENCLOUD_MODE_SECE)
    _setup = new EceSetup(_cfg);
#elif defined(LIBENCLOUD_MODE_VPN)
    _setup = new VpnSetup(_cfg);
#endif
    LIBENCLOUD_ERR_IF (_setup->setNetworkAccessManager(_qnam));
    LIBENCLOUD_ERR_IF (_setup == NULL);

    _setupObj = _setup;
    LIBENCLOUD_ERR_IF (_setupObj == NULL);

    // error signal handling
    connect(_setupObj, SIGNAL(error(libencloud::Error)), 
            this, SLOT(_errorReceived(libencloud::Error)));

    // authentication signal handling
    connect(this, SIGNAL(authSupplied(libencloud::Auth)), 
           _setupObj, SIGNAL(authSupplied(libencloud::Auth)));
    connect(_setupObj, SIGNAL(authRequired(libencloud::Auth::Id, QVariant)), 
           this, SLOT(_authRequired(libencloud::Auth::Id, QVariant)));
    connect(_setupObj, SIGNAL(authChanged(libencloud::Auth)),
           this, SIGNAL(authSupplied(libencloud::Auth)));

    // progress signal handling
    connect(_setupObj, SIGNAL(progress(Progress)), 
            this, SLOT(_progressReceived(Progress)));

    // need message signal forwarding
    connect(_setupObj, SIGNAL(need(QString, QVariant)), 
            this, SIGNAL(need(QString, QVariant)));

    // server configuration handling and forwarding
    connect(_setupObj, SIGNAL(serverConfigSupply(QVariant)), 
            this, SLOT(_serverConfigReceived(QVariant)));
    connect(_setupObj, SIGNAL(serverConfigSupply(QVariant)), 
            this, SIGNAL(serverConfigSupply(QVariant)));

    // setup completion handling
    connect(_setupObj, SIGNAL(completed()), 
            this, SLOT(_setupCompleted()));
    connect(_setupObj, SIGNAL(stopped()), 
            this, SLOT(_setupStopped()));

    return 0;
err:
    return ~0;
}

int Core::_initCloud ()
{
    LIBENCLOUD_TRACE;

    // reset pre-existing instances
    LIBENCLOUD_DELETE_LATER(_cloud);

    _cloud = new Cloud(_cfg);
    LIBENCLOUD_ERR_IF (_cloud == NULL);
    _cloud->setSetup(_setup);

    _cloudObj = _cloud;
    LIBENCLOUD_ERR_IF (_cloudObj == NULL);

    // error signal handling
    connect(_cloudObj, SIGNAL(error(libencloud::Error)), 
            this, SLOT(_errorReceived(libencloud::Error)));

    // authentication signal handling
    connect(this, SIGNAL(authSupplied(libencloud::Auth)), 
           _cloudObj, SIGNAL(authSupplied(libencloud::Auth)));
    connect(_cloudObj, SIGNAL(authRequired(libencloud::Auth::Id)), 
           this, SLOT(_authRequired(libencloud::Auth::Id)));

    // state changes forwarding for connecting/connected states
    connect(_cloudObj, SIGNAL(stateChanged(State)), 
            this, SIGNAL(stateChanged(State)));
    connect(this, SIGNAL(stateChanged(State)), 
            this, SLOT(_stateChanged(State)));
    connect(_cloudObj, SIGNAL(fallback(bool)), 
            this, SLOT(_fallback(bool)));

    // progress signal handling
    connect(_cloudObj, SIGNAL(progress(Progress)), 
            this, SLOT(_progressReceived(Progress)));

    // need message signal forwarding
    connect(_cloudObj, SIGNAL(need(QString, QVariant)), 
            this, SIGNAL(need(QString, QVariant)));

    return 0;
err:
    return ~0;
}

// The Cloud API client is used to forward actions to GUI
int Core::_initApi ()
{
    LIBENCLOUD_TRACE;

    _cloudApi = new libencloud::CloudApi;
    LIBENCLOUD_ERR_IF (_cloudApi == NULL);

    // Encloud Service forwards action requests to Gui via Cloud API
    if (_mode == EncloudMode)
        connect(this, SIGNAL(actionRequest(QString, libencloud::Params)), 
                _cloudApi, SLOT(actionRequest(QString, libencloud::Params)));

    // else if (mode == GuiMode) signal is caught by Gui

    return 0;
err:
    return ~0;
}

int Core::_initFsm ()
{
    LIBENCLOUD_TRACE;

    // clear previous FSM
    _fsm.stop();
    Q_FOREACH(QAbstractState *s, _fsm.configuration())
        _fsm.removeState(s);
    
#if !defined(LIBENCLOUD_DISABLE_SETUP)
    LIBENCLOUD_DBG("setupState: " << _setupState);
    disconnect(_setupState, NULL, NULL, NULL);
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
    disconnect(_cloudState, NULL, NULL, NULL);
    connect(_cloudState, SIGNAL(entered()), 
            this, SLOT(_stateEntered()));
    connect(_cloudState, SIGNAL(exited()), 
            this, SLOT(_stateExited()));
    _fsm.addState(_cloudState);
#endif

    Q_FOREACH(QAbstractTransition *t, _setupState->transitions())
        _setupState->removeTransition(t);
#if !defined(LIBENCLOUD_DISABLE_SETUP) && !defined(LIBENCLOUD_DISABLE_CLOUD)
    _setupState->addTransition(_setupObj, SIGNAL(completed()), _cloudState);
#endif

    _fsm.setInitialState(_initialState);

    return 0;
}

// Init local objects
int Core::_init ()
{
    connect(&_clientWatchdog, SIGNAL(down()), this, SLOT(_clientDown()));

    _qnam = new QNetworkAccessManager;
    LIBENCLOUD_ERR_IF (_qnam == NULL);

    _networkManager = new NetworkManager;
    LIBENCLOUD_ERR_IF (_networkManager == NULL);

    return 0;
err:
    return ~0;
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

#ifndef Q_OS_WINCE
/* Subject name settings from JSON CSR template */
static int _libencloud_context_name_cb (X509_NAME *n, void *arg)
{
#undef __LIBENCLOUD_MSG
#define __LIBENCLOUD_MSG __LIBENCLOUD_PRINT

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
#endif
