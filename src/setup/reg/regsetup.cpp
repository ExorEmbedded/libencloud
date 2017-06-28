#define LIBENCLOUD_DISABLE_TRACE  // disable heavy tracing
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/reg/regsetup.h>

namespace libencloud {

RegSetup::RegSetup (Config *cfg)
    : SetupInterface(cfg)
{
    LIBENCLOUD_TRACE;

    _initFsm();
    connect(&_retry, SIGNAL(timeout()), SLOT(_onRetryTimeout()));
    connect(_errorState, SIGNAL(entered()), this, SLOT(_onErrorState()));

    _initMsg(_regMsg);
    connect(&_regMsg, SIGNAL(error(libencloud::Error)),
            this, SLOT(_onError(libencloud::Error)));
    connect(&_regMsg, SIGNAL(processed()),
            this, SLOT(_onProcessed()));
    connect(&_regMsg, SIGNAL(authRequired(libencloud::Auth::Id, QVariant)),
            this, SIGNAL(authRequired(libencloud::Auth::Id, QVariant)));
    connect(this, SIGNAL(authSupplied(libencloud::Auth)),
            &_regMsg, SLOT(authSupplied(libencloud::Auth)));
    connect(&_regMsg, SIGNAL(authChanged(libencloud::Auth)),
            this, SIGNAL(authChanged(libencloud::Auth)));

    connect(_regMsgState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_regMsgState, SIGNAL(entered()), &_regMsg, SLOT(process()));
    connect(_regMsgState, SIGNAL(exited()), this, SLOT(_stateExited()));
    
    _initMsg(_closeMsg);
    connect(this, SIGNAL(authSupplied(libencloud::Auth)),
            &_closeMsg, SLOT(authSupplied(libencloud::Auth)));
    connect(&_regMsg, SIGNAL(authChanged(libencloud::Auth)),
            &_closeMsg, SLOT(authSupplied(libencloud::Auth)));
    connect(&_closeMsg, SIGNAL(processed()),
            this, SLOT(_onCloseProcessed()));
}

int RegSetup::start ()
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

int RegSetup::stop (bool reset, bool close)
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

const VpnConfig *RegSetup::getVpnConfig () const
{
    if (m_setupEnabled)
        return _regMsg.getVpnConfig();
    else
        return &_vpnConfig;
}

const VpnConfig *RegSetup::getFallbackVpnConfig () const
{
    if (m_setupEnabled)
        return _regMsg.getFallbackVpnConfig();
    else
        return &_vpnFallbackConfig;
}

int RegSetup::getTotalSteps() const
{
    return StateLast - StateFirst + 1;
}

//
// private slots
//

void RegSetup::_stateEntered ()
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

void RegSetup::_stateExited ()
{
    QState *state = qobject_cast<QState *>(sender());
    Progress p = _stateToProgress(state);

    LIBENCLOUD_DBG("state: " << state << " (" << p.getDesc() << ")");

    _isError = false;
}

void RegSetup::_onProcessed ()
{
    LIBENCLOUD_TRACE;

    emit completed();
}

void RegSetup::_onCloseProcessed ()
{
    LIBENCLOUD_TRACE;

    emit stopped();
}

void RegSetup::_onErrorState ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state);

    _isError = true;

    switch (_error.getCode())
    {
        case Error::CodeUnconfigured:
            break;
        // keep on retrying
        default:
            _errorState->addTransition(this, SIGNAL(retry()), _previousState);
            if (_cfg->config.autoretry)
                _retry.start();
            break;
    }
}

void RegSetup::_onError (const libencloud::Error &err)
{
    LIBENCLOUD_DBG(err.toString());

    emit error((_error = err));
}

void RegSetup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    if (_cfg->config.autoretry)
        emit retry();
}

//
// private methods
//
int RegSetup::_initFsm ()
{
    m_setupEnabled = _cfg->config.setupEnabled;

    LIBENCLOUD_DBG("setupEnabled: " << m_setupEnabled);

    _isError = false;
    _errorState = &_errorSt;
    if (m_setupEnabled)
        _regMsgState = &_regMsgSt;
    else
        _regMsgState = NULL;
    _finalState = &_finalSt;

    if (m_setupEnabled)
    {
        _initMsg(_regMsg);
        _regMsgState->addTransition(&_regMsg, SIGNAL(error(libencloud::Error)), _errorState);
        _regMsgState->addTransition(&_regMsg, SIGNAL(processed()), _finalState);
    }

    _fsm.addState(_errorState);
    if (m_setupEnabled)
        _fsm.addState(_regMsgState);
    _fsm.addState(_finalState);

    if (m_setupEnabled)
        _fsm.setInitialState(_regMsgState);
    else
        _fsm.setInitialState(_finalState);

    _previousState = qobject_cast<QState*> (_fsm.initialState());

    return 0;
}

// cached message data is cleared only if reset=true (default)
int RegSetup::_deinitFsm (bool reset)
{
    _fsm.removeState(_finalState);
    if (m_setupEnabled)
        _fsm.removeState(_regMsgState);
    _fsm.removeState(_errorState);

    if (m_setupEnabled)
        Q_FOREACH(QAbstractTransition *transition, _regMsgState->transitions())
            _regMsgState->removeTransition(transition);

    if (reset)
        _regMsg.clear();
    _clear();

    return 0;
}

int RegSetup::_initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);

    return 0;
}

Progress RegSetup::_stateToProgress (QState *state)
{
    Progress p;

    p.setTotal(getTotalSteps());

    if (state == _errorState)
    {
        p.setStep(-1);
        p.setDesc("Setup Error State");
    } 
    else if (state == _regMsgState)
    {
        p.setStep(StateRegMsg);
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
void RegSetup::_clear ()
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
