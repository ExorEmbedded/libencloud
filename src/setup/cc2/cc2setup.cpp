#define LIBENCLOUD_DISABLE_TRACE  // disable heavy tracing
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/cc2/cc2setup.h>

namespace libencloud {

CC2Setup::CC2Setup (Config *cfg)
    : SetupInterface(cfg)
    , _qccSetup(cfg)
    , _qccConsecutiveErrors(0)
{
    LIBENCLOUD_TRACE;

    _initFsm();

    _retry.setBackoff(2);
    connect(&_retry, SIGNAL(timeout()), SLOT(_onRetryTimeout()));

    connect(_errorState, SIGNAL(entered()), this, SLOT(_onErrorState()));

    _initMsg(_cc2Msg);
    connect(&_cc2Msg, SIGNAL(error(libencloud::Error)),
            this, SLOT(_onError(libencloud::Error)));
    connect(&_cc2Msg, SIGNAL(processed()),
            this, SLOT(_onProcessed()));
    connect(&_cc2Msg, SIGNAL(authRequired(libencloud::Auth::Id, QVariant)),
            this, SIGNAL(authRequired(libencloud::Auth::Id, QVariant)));
    connect(this, SIGNAL(authSupplied(libencloud::Auth)),
            &_cc2Msg, SLOT(authSupplied(libencloud::Auth)));
    connect(&_cc2Msg, SIGNAL(authChanged(libencloud::Auth)),
            this, SIGNAL(authChanged(libencloud::Auth)));

    connect(_cc2MsgState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_cc2MsgState, SIGNAL(entered()), &_cc2Msg, SLOT(process()));
    connect(_cc2MsgState, SIGNAL(exited()), this, SLOT(_stateExited()));
    
    connect(_qccSetupState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_qccSetupState, SIGNAL(entered()), &_qccSetup, SLOT(start()));
    connect(_qccSetupState, SIGNAL(exited()), this, SLOT(_stateExited()));
    connect(&_cc2Msg, SIGNAL(authChanged(libencloud::Auth)),
            &_qccSetup, SIGNAL(authSupplied(libencloud::Auth)));
    connect(&_qccSetup, SIGNAL(completed()), this, SIGNAL(completed()));
    connect(&_qccSetup, SIGNAL(error(libencloud::Error)), this, SLOT(_onQccError(libencloud::Error)));

    
    _initMsg(_closeMsg);
    connect(this, SIGNAL(authSupplied(libencloud::Auth)),
            &_closeMsg, SLOT(authSupplied(libencloud::Auth)));
    connect(&_cc2Msg, SIGNAL(authChanged(libencloud::Auth)),
            &_closeMsg, SLOT(authSupplied(libencloud::Auth)));
    connect(&_closeMsg, SIGNAL(processed()),
            this, SLOT(_onCloseProcessed()));
}

int CC2Setup::start ()
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

int CC2Setup::stop (bool reset, bool close)
{
    LIBENCLOUD_DBG("reset: " << reset << ", close: " << close);

    _retry.stop();
    _fsm.stop();
    _qccSetup.stop(reset, close);
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

const VpnConfig *CC2Setup::getVpnConfig () const
{
    return _qccSetup.getVpnConfig();
}

const VpnConfig *CC2Setup::getFallbackVpnConfig () const
{
    return _qccSetup.getFallbackVpnConfig();
}

int CC2Setup::getTotalSteps() const
{
    return StateLast - StateFirst + 1;
}

//
// private slots
//

void CC2Setup::_stateEntered ()
{
    QState *state = qobject_cast<QState *>(sender());
    Progress p = _stateToProgress(state);

    LIBENCLOUD_DBG("state: " << state << " (" << p.getDesc() << ") " <<
            " previousState: " << _stateToProgress(_previousState).getDesc());

    if (!_isError)
        _retry.stop();

    _previousState = state;

    emit progress(p);

    if (state == _finalState)
        emit completed();
}

void CC2Setup::_stateExited ()
{
    QState *state = qobject_cast<QState *>(sender());
    Progress p = _stateToProgress(state);

    LIBENCLOUD_DBG("state: " << state << " (" << p.getDesc() << ")");

    _isError = false;
}

void CC2Setup::_onProcessed ()
{
    LIBENCLOUD_TRACE;

}

void CC2Setup::_onCloseProcessed ()
{
    LIBENCLOUD_TRACE;

    emit stopped();
}

void CC2Setup::_onErrorState ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state);

    _isError = true;

    switch (_error.getCode())
    {
        // critical errors that require user intervention
        case Error::CodeBadActivation:
        case Error::CodeUnregActivation:
        case Error::CodeUnconfigured:
            break;
        // keep on retrying
        default:
            _errorState->addTransition(this, SIGNAL(retry()), _previousState);
            _retry.start();
            break;
    }
}

void CC2Setup::_onError (const libencloud::Error &err)
{
    LIBENCLOUD_DBG(err.toString());

    emit error((_error = err));
}

void CC2Setup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    emit retry();
}

void CC2Setup::_onQccError(const Error &error)
{
    _qccConsecutiveErrors++;
    if (_qccConsecutiveErrors > 3) {
        _qccConsecutiveErrors = 0;
        _qccSetup.stop(true, false);
        LIBENCLOUD_ERR("Retry pairing after consecutive errors in setup phase");
        emit retryAfterConsecutiveErrorsInQccSetup();
    }
}

//
// private methods
//
int CC2Setup::_initFsm ()
{
    m_setupEnabled = _cfg->config.setupEnabled;

    LIBENCLOUD_DBG("setupEnabled: " << m_setupEnabled);

    _isError = false;
    _errorState = &_errorSt;
    if (m_setupEnabled) {
        _cc2MsgState = &_cc2MsgSt;
        _qccSetupState = &_qccSetupSt;
    } else {
        _cc2MsgState = NULL;
        _qccSetupState = NULL;
    }
    _finalState = &_finalSt;

    if (m_setupEnabled)
    {
        _initMsg(_cc2Msg);
        _cc2MsgState->addTransition(&_cc2Msg, SIGNAL(error(libencloud::Error)), _errorState);
        _cc2MsgState->addTransition(&_cc2Msg, SIGNAL(processed()), _qccSetupState);

        // do not connect error (has its own retry logic):  _qccSetupState->addTransition(&_qccSetup, SIGNAL(error(libencloud::Error)), _errorState);
        _qccSetupState->addTransition(&_qccSetup, SIGNAL(completed()), _finalState);
        _qccSetupState->addTransition(this, SIGNAL(retryAfterConsecutiveErrorsInQccSetup()), _cc2MsgState);
    }

    _fsm.addState(_errorState);
    if (m_setupEnabled) {
        _fsm.addState(_cc2MsgState);
         _fsm.addState(_qccSetupState);
    }
    _fsm.addState(_finalState);

    if (m_setupEnabled)
        _fsm.setInitialState(_cc2MsgState);
    else
        _fsm.setInitialState(_finalState);

    _previousState = qobject_cast<QState*> (_fsm.initialState());

    return 0;
}

// cached message data is cleared only if reset=true (default)
int CC2Setup::_deinitFsm (bool reset)
{
    _fsm.removeState(_finalState);
    if (m_setupEnabled) {
        _fsm.removeState(_cc2MsgState);
        _fsm.removeState(_qccSetupState);
    }
    _fsm.removeState(_errorState);

    if (m_setupEnabled) {
        Q_FOREACH(QAbstractTransition *transition, _cc2MsgState->transitions())
            _cc2MsgState->removeTransition(transition);
        Q_FOREACH(QAbstractTransition *transition, _qccSetupState->transitions())
            _qccSetupState->removeTransition(transition);
    }

    if (reset) {
        _cc2Msg.clear();
    }
    _clear();

    return 0;
}

int CC2Setup::_initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);

    return 0;
}

Progress CC2Setup::_stateToProgress (QState *state)
{
    Progress p;

    p.setTotal(getTotalSteps());

    if (state == _errorState)
    {
        p.setStep(-1);
        p.setDesc("Setup Error State");
    } 
    else if (state == _cc2MsgState)
    {
        p.setStep(StateCC2Msg);
        p.setDesc(tr("Retrieving credentials"));
    }
    else if (state == _qccSetupState)
    {
        p.setStep(StateQCCStarted);
        p.setDesc(tr("Retrieving Configuration from Switchboard via qcc"));
    }
    else if (state == _finalState)
    {
        _qccConsecutiveErrors = 0;
        p.setStep(StateQCCCompleted);
        p.setDesc(tr("Received Configuration from Switchboard via qcc"));
    }
    else
    {
        p.setStep(StateInvalid);
    }

    return p;
}

// Clear all generated/temporary data (e.g. configuration downloaded form Switchboard),
// without clearing user-configured profile data
void CC2Setup::_clear ()
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
