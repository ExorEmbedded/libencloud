#include <QtCore/qmath.h>
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/ece/setup.h>

namespace libencloud {

/**
 *   retrInfoState -> retrCertState -> retrConfState
 *          |               |               | 
 *          V               V               V 
 *          +---------------+---------------+
 *                          |
 *                          V        RETRY_TIMEOUT * backoff
 *                      errorState  --------------------------> previousState
 */
EceSetup::EceSetup ()
    : _initialState(NULL)
    , _previousState(NULL)
    , _error(false)
    , _backoff(1)
{
    LIBENCLOUD_TRACE;
}

int EceSetup::initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);
    msg.setClient(&_client);
    msg.init();

    return 0;
}

int EceSetup::init ()
{
    LIBENCLOUD_TRACE;

    _initialState = &_retrInfoState;
    _previousState = _initialState;

    LIBENCLOUD_DBG("retrInfoState: " << &_retrInfoState);
    LIBENCLOUD_DBG("retrCertState: " << &_retrCertState);
    LIBENCLOUD_DBG("errorState: " << &_errorState);

    // failures result in retry of previous state (with backoff)
    connect(&_errorState, SIGNAL(entered()), this, SLOT(_onError()));

    initMsg(_retrInfoMsg);
    connect(&_retrInfoState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(&_retrInfoState, SIGNAL(entered()), &_retrInfoMsg, SLOT(process()));
    connect(&_retrInfoState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrInfoState.addTransition(&_retrInfoMsg, SIGNAL(error()), &_errorState);
    _retrInfoState.addTransition(&_retrInfoMsg, SIGNAL(processed()), &_retrCertState);

    initMsg(_retrCertMsg);
    connect(&_retrCertState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(&_retrCertState, SIGNAL(entered()), &_retrCertMsg, SLOT(process()));
    connect(&_retrCertState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrCertState.addTransition(&_retrCertMsg, SIGNAL(error()), &_errorState);
    _retrCertState.addTransition(&_retrCertMsg, SIGNAL(processed()), &_retrConfState);

    initMsg(_retrConfMsg);
    connect(&_retrConfState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(&_retrConfState, SIGNAL(entered()), &_retrConfMsg, SLOT(process()));
    connect(&_retrConfState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrCertState.addTransition(&_retrCertMsg, SIGNAL(error()), &_errorState);
    _retrCertState.addTransition(&_retrCertMsg, SIGNAL(processed()), &_checkExpiryState);

    connect(&_checkExpiryState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(&_checkExpiryState, SIGNAL(exited()), this, SLOT(_stateExited()));

    _fsm.addState(&_errorState);
    _fsm.addState(&_retrInfoState);
    _fsm.addState(&_retrCertState);
    _fsm.addState(&_retrConfState);
    _fsm.addState(&_checkExpiryState);

    _fsm.setInitialState(_initialState);

    return 0;
}

int EceSetup::start ()
{
    LIBENCLOUD_TRACE;

    //retrSbInfo();
    _fsm.start();

    return 0;
}

int EceSetup::retrSbInfo ()
{
    LIBENCLOUD_TRACE;

    return 0;
}

//
// private slots
//

void EceSetup::_stateEntered ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("sender: " << state << " previousState: " << _previousState);

    if (!_error)
        _backoff = 1;

    _previousState = state;
}

void EceSetup::_stateExited ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("sender: " << state);

    _error = false;
}

void EceSetup::_onError ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("sender: " << state << " backoff: " << QString::number(_backoff));

    _error = true;

    QTimer::singleShot(qPow(LIBENCLOUD_RETRY_TIMEOUT, _backoff) * 1000,
            this, SLOT(_onRetryTimeout()));
    _backoff++;

    //_previousState = state;
    // remove previous transitions?

    _errorState.addTransition(this, SIGNAL(retry()), _previousState);
}

void EceSetup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    // <TEST> disable retry
#if 1
    emit retry();
#endif
}


} // namespace libencloud
