#define LIBENCLOUD_DISABLE_TRACE  // disable heavy tracing
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/qcc/qccsetup.h>

namespace libencloud {

QccSetup::QccSetup (Config *cfg)
    : SetupInterface(cfg)
    , _initialState(&_setupMsgSt)
    , _previousState(_initialState)
    , _errorState(&_errorSt)
    , _setupMsgState(&_setupMsgSt)
    , _finalState(&_finalSt)
    , _isError(false)
{
    LIBENCLOUD_TRACE;

    _initFsm();

    connect(&_retry, SIGNAL(timeout()), SLOT(_onRetryTimeout()));
}

int QccSetup::start ()
{
    LIBENCLOUD_TRACE;

    _clear();

    _fsm.start();

    return 0;
}

int QccSetup::stop ()
{
    LIBENCLOUD_TRACE;

    _retry.stop();
    _fsm.stop();

    _clear();
    _setupMsg.clear();
    _initMsg(_setupMsg);

    return 0;
}

const VpnConfig *QccSetup::getVpnConfig () const
{
    return _setupMsg.getVpnConfig();
}

const VpnConfig *QccSetup::getFallbackVpnConfig () const
{
    return _setupMsg.getFallbackVpnConfig();
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

void QccSetup::_onErrorState ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state);

    _isError = true;

    switch (_error.getCode())
    {
        // keep on retrying
        default:
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
    // failures result in retry of previous state (with backoff)
    connect(_errorState, SIGNAL(entered()), this, SLOT(_onErrorState()));
    
    _initMsg(_setupMsg);
    connect(&_setupMsg, SIGNAL(error(libencloud::Error)),
            this, SLOT(_onError(libencloud::Error)));
    connect(&_setupMsg, SIGNAL(need(QString)),
            this, SIGNAL(need(QString)));
    connect(&_setupMsg, SIGNAL(processed()),
            this, SLOT(_onProcessed()));
    connect(&_setupMsg, SIGNAL(authRequired(Auth::Id)),
            this, SIGNAL(authRequired(Auth::Id)));
    connect(this, SIGNAL(authSupplied(Auth)),
            &_setupMsg, SLOT(authSupplied(Auth)));
    connect(&_setupMsg, SIGNAL(serverConfigSupply(QVariant)),
            this, SIGNAL(serverConfigSupply(QVariant)));
    connect(_setupMsgState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_setupMsgState, SIGNAL(entered()), &_setupMsg, SLOT(process()));
    connect(_setupMsgState, SIGNAL(exited()), this, SLOT(_stateExited()));

    _setupMsgState->addTransition(&_setupMsg, SIGNAL(error(libencloud::Error)), _errorState);
    _setupMsgState->addTransition(&_setupMsg, SIGNAL(processed()), _finalState);

    _fsm.addState(_errorState);
    _fsm.addState(_setupMsgState);
    _fsm.addState(_finalState);

    _fsm.setInitialState(_initialState);

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

// Clear all generated data
void QccSetup::_clear ()
{
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
