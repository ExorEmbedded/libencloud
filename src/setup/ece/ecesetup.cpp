#include <QtCore/qmath.h>
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/ece/ecesetup.h>

namespace libencloud {

//
// public methods
//

EceSetup::EceSetup (Config *cfg)
    : SetupInterface(cfg)
    , _initialState(&_retrInfoSt)
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

    _initFsm();
}

int EceSetup::start ()
{
    LIBENCLOUD_TRACE;

    _fsm.start();

    return 0;
}

int EceSetup::stop ()
{
    LIBENCLOUD_TRACE;

    _fsm.stop();
    // TODO handle stopped()

    return 0;
}

const VpnConfig *EceSetup::getVpnConfig ()
{
    return _retrConfMsg.getVpnConfig();
}

int EceSetup::getTotalSteps() const
{
    return StateLast - StateFirst + 1;
}

//
// public slots
//

//
// private slots
//

void EceSetup::_stateEntered ()
{
    QState *state = qobject_cast<QState *>(sender());
    Progress p = _stateToProgress(state);

    LIBENCLOUD_DBG("state: " << state << " (" << p.getDesc() << ") " <<
            " previousState: " << _previousState);

    if (!_error)
        _backoff = 1;

    _previousState = state;

    emit progress(p);

    if (state == _completedState)
        emit completed();
}

void EceSetup::_stateExited ()
{
    QState *state = qobject_cast<QState *>(sender());
    Progress p = _stateToProgress(state);

    LIBENCLOUD_DBG("state: " << state << " (" << p.getDesc() << ")");

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

    emit error();
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

int EceSetup::_initFsm ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DBG("errorState: " << _errorState);
    LIBENCLOUD_DBG("retrInfoState: " << _retrInfoState);
    LIBENCLOUD_DBG("retrCertState: " << _retrCertState);
    LIBENCLOUD_DBG("retrConfState: " << _retrConfState);

    // failures result in retry of previous state (with backoff)
    connect(_errorState, SIGNAL(entered()), this, SLOT(_onError()));

    _initMsg(_retrInfoMsg);
    connect(&_retrInfoMsg, SIGNAL(need(QString)), this, SIGNAL(need(QString)));
    // only SECE needs license, which is received via setup module
#ifdef LIBENCLOUD_MODE_SECE
    connect(this, SIGNAL(licenseForward(QUuid)), &_retrInfoMsg, SLOT(licenseReceived(QUuid)));
#endif
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

int EceSetup::_initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);
    msg.setClient(&_client);
    msg.init();

    return 0;
}

Progress EceSetup::_stateToProgress (QState *state)
{
    Progress p;

    p.setTotal(getTotalSteps());

    if (state == _errorState)
    {
        p.setStep(-1);
        p.setDesc("Setup Error State");
    }
    else if (state == _retrInfoState)
    {
        p.setStep(StateRetrInfo);
        p.setDesc("Retrieving Info from Switchboard");
    }
    else if (state == _retrCertState)
    {
        p.setStep(StateRetrCert);
        p.setDesc("Retrieving Certificate from Switchboard");
    }
    else if (state == _retrConfState)
    {
        p.setStep(StateRetrConf);
        p.setDesc("Retrieving Configuration from Switchboard");
    }
    else if (state == _checkExpiryState)
    {
        p.setStep(StateCheckExpiry);
        p.setDesc("Checking Certificate/License Expiry");
    }
    else
    {
        p.setStep(StateInvalid);
    }

    return p;
}

} // namespace libencloud
