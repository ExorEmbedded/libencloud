#define LIBENCLOUD_DISABLE_TRACE  // disable heavy tracing
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/qcc/qccsetup.h>

namespace libencloud {

QccSetup::QccSetup (Config *cfg)
    : SetupInterface(cfg)
{
    LIBENCLOUD_TRACE;

    _initFsm();
    connect(&_retry, SIGNAL(timeout()), SLOT(_onRetryTimeout()));
    connect(_errorState, SIGNAL(entered()), this, SLOT(_onErrorState()));

    _initMsg(_setupMsg);
    connect(&_setupMsg, SIGNAL(error(libencloud::Error)),
            this, SLOT(_onError(libencloud::Error)));
    connect(&_setupMsg, SIGNAL(processed()),
            this, SLOT(_onProcessed()));
    connect(&_setupMsg, SIGNAL(authRequired(Auth::Id, QVariant)),
            this, SIGNAL(authRequired(Auth::Id, QVariant)));
    connect(&_setupMsg, SIGNAL(serverConfigSupply(QVariant)),
            this, SIGNAL(serverConfigSupply(QVariant)));
    connect(this, SIGNAL(authSupplied(Auth)),
            &_setupMsg, SLOT(authSupplied(Auth)));

    connect(_setupMsgState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_setupMsgState, SIGNAL(entered()), &_setupMsg, SLOT(process()));
    connect(_setupMsgState, SIGNAL(exited()), this, SLOT(_stateExited()));

    _initMsg(_closeMsg);
    connect(this, SIGNAL(authSupplied(Auth)),
            &_closeMsg, SLOT(authSupplied(Auth)));
    connect(&_closeMsg, SIGNAL(processed()),
            this, SLOT(_onCloseProcessed()));
}

int QccSetup::start ()
{
    LIBENCLOUD_TRACE;

    _initFsm();
    _closeMsg.setNetworkAccessManager(getNetworkAccessManager());

    if (!m_setupEnabled)
    {
        // critical
        if (_vpnConfig.fromFile(_cfg->config.vpnConfPath.absoluteFilePath(), false))
            LIBENCLOUD_EMIT_ERR (error(Error(tr("No valid VPN configuration found!"))));

        // non-critical
        if (_vpnFallbackConfig.fromFile(_cfg->config.fallbackVpnConfPath.absoluteFilePath(), false))
            LIBENCLOUD_DBG("No valid VPN fallback configuration found");
    }

    _fsm.start();

    if (_fsm.initialState() == _finalState)
        emit completed();

    return 0;

err:
    return ~0;
}

int QccSetup::stop (bool reset, bool close)
{
    LIBENCLOUD_DBG("reset: " << reset << ", close: " << close);

    _retry.stop();
    _fsm.stop();
    _deinitFsm(reset);

    if (!m_setupEnabled || (reset && !close))
        emit stopped();
    else if (m_setupEnabled && reset && close)
        // triggers _onCloseProcessed() upon completion
        LIBENCLOUD_ERR_IF (_closeMsg.process());  

    return 0;
err:
    return ~0;
}

const VpnConfig *QccSetup::getVpnConfig () const
{
    if (m_setupEnabled)
        return _setupMsg.getVpnConfig();
    else
        return &_vpnConfig;
}

const VpnConfig *QccSetup::getFallbackVpnConfig () const
{
    if (m_setupEnabled)
        return _setupMsg.getFallbackVpnConfig();
    else
        return &_vpnFallbackConfig;
}

int QccSetup::getTotalSteps() const
{
    return StateLast - StateFirst + 1;
}

//
// private slots
//

void QccSetup::_stateEntered ()
{
    QState *state = qobject_cast<QState *>(sender());
    Progress p = _stateToProgress(state);

    LIBENCLOUD_DBG("state: " << state << " (" << p.getDesc() << ") " <<
            " previousState: " << _previousState);

    if (!_isError)
        _retry.stop();

    _previousState = state;

    emit progress(p);

    if (state == _finalState)
        emit completed();
}

void QccSetup::_stateExited ()
{
    QState *state = qobject_cast<QState *>(sender());
    Progress p = _stateToProgress(state);

    LIBENCLOUD_DBG("state: " << state << " (" << p.getDesc() << ")");

    _isError = false;
}

void QccSetup::_onProcessed ()
{
    LIBENCLOUD_TRACE;

    emit completed();
}

void QccSetup::_onCloseProcessed ()
{
    LIBENCLOUD_TRACE;

    emit stopped();
}

void QccSetup::_onErrorState ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state);

    _isError = true;

    switch (_error.getCode())
    {
        // keep on retrying
        default:
            qDeleteAll(_errorState->transitions());
            _errorState->transitions().clear();
            _errorState->addTransition(this, SIGNAL(retry()), _previousState);
            if (_cfg->config.autoretry)
                _retry.start();
            break;
    }
}

void QccSetup::_onError (const libencloud::Error &err)
{
    LIBENCLOUD_DBG(err.toString());

    emit error((_error = err));
}

void QccSetup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    if (_cfg->config.autoretry)
        emit retry();
}

//
// private methods
//
int QccSetup::_initFsm ()
{
    m_setupEnabled = _cfg->config.setupEnabled;

    LIBENCLOUD_DBG("setupEnabled: " << m_setupEnabled);

    _isError = false;
    _errorState = &_errorSt;
    if (m_setupEnabled)
        _setupMsgState = &_setupMsgSt;
    else
        _setupMsgState = NULL;
    _finalState = &_finalSt;

    if (m_setupEnabled)
    {
        _initMsg(_setupMsg);
        _setupMsgState->addTransition(&_setupMsg, SIGNAL(error(libencloud::Error)), _errorState);
        _setupMsgState->addTransition(&_setupMsg, SIGNAL(processed()), _finalState);
    }

    _fsm.addState(_errorState);
    if (m_setupEnabled)
        _fsm.addState(_setupMsgState);
    _fsm.addState(_finalState);

    if (m_setupEnabled)
        _fsm.setInitialState(_setupMsgState);
    else
        _fsm.setInitialState(_finalState);

    _previousState = qobject_cast<QState*> (_fsm.initialState());

    return 0;
}

// cached message data is cleared only if reset=true (default)
int QccSetup::_deinitFsm (bool reset)
{
    _fsm.removeState(_finalState);
    if (m_setupEnabled)
        _fsm.removeState(_setupMsgState);
    _fsm.removeState(_errorState);

    if (m_setupEnabled)
        Q_FOREACH(QAbstractTransition *transition, _setupMsgState->transitions())
            _setupMsgState->removeTransition(transition);

    if (reset)
        _setupMsg.clear();
    _clear();

    return 0;
}

int QccSetup::_initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);

    return 0;
}

Progress QccSetup::_stateToProgress (QState *state)
{
    Progress p;

    p.setTotal(getTotalSteps());

    if (state == _errorState)
    {
        p.setStep(-1);
        p.setDesc("Setup Error State");
    } 
    else if (state == _setupMsgState)
    {
        p.setStep(StateSetupMsg);
        p.setDesc(tr("Retrieving Configuration from Switchboard"));
    }
    else if (state == _finalState)
    {
        p.setStep(StateReceived);
        p.setDesc(tr("Received Configuration from Switchboard"));
    }
    else
    {
        p.setStep(StateInvalid);
    }

    return p;
}

// Clear all generated/temporary data (e.g. configuration downloaded form Switchboard),
// without clearing user-configured profile data
void QccSetup::_clear ()
{
    _vpnConfig.clear();
    _vpnFallbackConfig.clear();

    if (!m_setupEnabled)
        return;

    if (QFile::exists(_cfg->config.sslOp.caPath.absoluteFilePath()))
        LIBENCLOUD_ERR_IF (!QFile::remove(_cfg->config.sslOp.caPath.absoluteFilePath()));
    if (QFile::exists(_cfg->config.vpnConfPath.absoluteFilePath()))
        LIBENCLOUD_ERR_IF (!QFile::remove(_cfg->config.vpnConfPath.absoluteFilePath()));
    if (QFile::exists(_cfg->config.fallbackVpnConfPath.absoluteFilePath()))
        LIBENCLOUD_ERR_IF (!QFile::remove(_cfg->config.fallbackVpnConfPath.absoluteFilePath()));
err:
    return;
}

} // namespace libencloud
