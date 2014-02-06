#include <QtCore/qmath.h>
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/ece/setup.h>

namespace libencloud {

/**
 *   retrInfoState -> retrCertState -> retrConfState
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

    LIBENCLOUD_DBG("initialState: " << _initialState);
    LIBENCLOUD_DBG("previousState: " << _previousState);
    LIBENCLOUD_DBG("errorState: " << &_errorState);

    // failures result in retry of previous state (with backoff)
    connect(&_errorState, SIGNAL(entered()), this, SLOT(_onError()));
    _errorState.addTransition(this, SIGNAL(retry()), _previousState);

    initMsg(_retrInfoMsg);
    connect(&_retrInfoState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(&_retrInfoState, SIGNAL(entered()), &_retrInfoMsg, SLOT(process()));
    connect(&_retrInfoState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrInfoState.addTransition(&_retrInfoMsg, SIGNAL(processed()), &_retrCertState);
    _retrInfoState.addTransition(&_retrInfoMsg, SIGNAL(error()), &_errorState);

    initMsg(_retrCertMsg);
    connect(&_retrCertState, SIGNAL(entered()), &_retrCertMsg, SLOT(process()));
#if 0
    retrCertState.addTransition(&retrCertMsg, SIGNAL(processed()), &retrConfState);
#endif

    //retrConfMsg.setConfig(_cfg);
    //retrConfState.addTransition(&retrConfMsg, SIGNAL(processed()), &checkExpiryState);

    _fsm.addState(&_errorState);
    _fsm.addState(&_retrInfoState);
    _fsm.addState(&_retrCertState);
#if 0
    _fsm.addState(&retrConfState);
#endif
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
    LIBENCLOUD_DBG("sender: " << sender());

    if (!_error)
        _backoff = 1;

    _previousState = qobject_cast<QState *>(sender());
}

void EceSetup::_stateExited ()
{
    LIBENCLOUD_DBG("sender: " << sender());

    _error = false;
}

void EceSetup::_onError ()
{
    LIBENCLOUD_TRACE;

    _error = true;

    QTimer::singleShot(qPow(LIBENCLOUD_RETRY_TIMEOUT, _backoff) * 1000, 
            this, SLOT(_onRetryTimeout()));
    _backoff++;
}

void EceSetup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    emit retry();
}


} // namespace libencloud
