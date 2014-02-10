#include <QtCore/qmath.h>
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/ece/setup.h>


namespace libencloud {

//
// public methods
//

EceSetup::EceSetup ()
    : _initialState(&_retrInfoSt)
    , _completedState(&_checkExpirySt)
    , _previousState(_initialState)
    , _errorState(&_errorSt)
    , _retrInfoState(&_retrInfoSt)
    , _retrCertState(&_retrCertSt)
    , _retrConfState(&_retrConfSt)
    , _checkExpiryState(&_checkExpirySt)
    , _error(false)
    , _backoff(1)
{
    LIBENCLOUD_TRACE;
}

int EceSetup::init ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DBG("retrInfoState: " << _retrInfoState);
    LIBENCLOUD_DBG("retrCertState: " << _retrCertState);
    LIBENCLOUD_DBG("errorState: " << _errorState);

    // failures result in retry of previous state (with backoff)
    connect(_errorState, SIGNAL(entered()), this, SLOT(_onError()));

    _initMsg(_retrInfoMsg);
    connect(_retrInfoState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_retrInfoState, SIGNAL(entered()), &_retrInfoMsg, SLOT(process()));
    connect(_retrInfoState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrInfoState->addTransition(&_retrInfoMsg, SIGNAL(error()), _errorState);
    _retrInfoState->addTransition(&_retrInfoMsg, SIGNAL(processed()), _retrCertState);

    _initMsg(_retrCertMsg);
    connect(_retrCertState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_retrCertState, SIGNAL(entered()), &_retrCertMsg, SLOT(process()));
    connect(_retrCertState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrCertState->addTransition(&_retrCertMsg, SIGNAL(error()), _errorState);
    _retrCertState->addTransition(&_retrCertMsg, SIGNAL(processed()), _retrConfState);

    _initMsg(_retrConfMsg);
    connect(_retrConfState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_retrConfState, SIGNAL(entered()), &_retrConfMsg, SLOT(process()));
    connect(_retrConfState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrConfState->addTransition(&_retrConfMsg, SIGNAL(error()), _errorState);
    _retrConfState->addTransition(&_retrConfMsg, SIGNAL(processed()), _checkExpiryState);

    connect(_checkExpiryState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_checkExpiryState, SIGNAL(exited()), this, SLOT(_stateExited()));

    _fsm.addState(_errorState);
    _fsm.addState(_retrInfoState);
    _fsm.addState(_retrCertState);
    _fsm.addState(_retrConfState);
    _fsm.addState(_checkExpiryState);

    _fsm.setInitialState(_initialState);

    return 0;
}

int EceSetup::start ()
{
    LIBENCLOUD_TRACE;

    _fsm.start();

    return 0;
}

const VpnConfig *EceSetup::getVpnConfig ()
{
    return _retrConfMsg.getVpnConfig();
}

//
// private slots
//

void EceSetup::_stateEntered ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state << " (" << _stateStr(state) << ") " <<
            " previousState: " << _previousState);

    if (!_error)
        _backoff = 1;

    _previousState = state;

    emit stateChanged(_stateStr(state));

    if (state == _completedState)
        emit completed();
}

void EceSetup::_stateExited ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state << " (" << _stateStr(state) << ")");

    _error = false;
}

void EceSetup::_onError ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state << " backoff: " << QString::number(_backoff));

    _error = true;

    QTimer::singleShot(qPow(LIBENCLOUD_RETRY_TIMEOUT, _backoff) * 1000,
            this, SLOT(_onRetryTimeout()));
    _backoff++;

    _errorState->addTransition(this, SIGNAL(retry()), _previousState);

    emit stateChanged(_stateStr(state));
}

void EceSetup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    // <TEST> disable retry
#if 1
    emit retry();
#endif
}

//
// private methods
//

int EceSetup::_initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);
    msg.setClient(&_client);
    msg.init();

    return 0;
}

QString EceSetup::_stateStr (QState *state)
{
    if (state == _errorState)
        return tr("Setup Error State");
    else if (state == _retrInfoState)
        return tr("Retrieving Info from Switchboard");
    else if (state == _retrCertState)
        return tr("Retrieving Certificate from Switchboard");
    else if (state == _retrConfState)
        return tr("Retrieving Configuration from Switchboard");
    else if (state == _checkExpiryState)
        return tr("Checking Certificate/License Expiry");
    else
        return "";
}

} // namespace libencloud
