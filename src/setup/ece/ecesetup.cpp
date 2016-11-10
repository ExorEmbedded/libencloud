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
    , _isError(false)
{
    LIBENCLOUD_TRACE;

    emit progress(Progress(tr("Initialising ECE Setup Module"), StateInit, getTotalSteps()));

    _initFsm();

    connect(&_retry, SIGNAL(timeout()), SLOT(_onRetryTimeout()));
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

    _retry.stop();
    _fsm.stop();

    return 0;
}

const VpnConfig *EceSetup::getVpnConfig () const
{
    return _retrConfMsg.getVpnConfig();
}

// Switchboard TODO?
const VpnConfig *EceSetup::getFallbackVpnConfig () const
{
    return NULL;
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

    if (!_isError)
        _retry.stop();

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

    _isError = false;
}

void EceSetup::_onErrorState ()
{
    QState *state = qobject_cast<QState *>(sender());

    LIBENCLOUD_DBG("state: " << state);

    _isError = true;

    switch (_error.getCode())
    {
#ifdef LIBENCLOUD_MODE_SECE
        // user intervention required for license entry (see setup API)
        case Error::CodeServerLicenseInvalid:
            break;
#endif

        // keep on retrying
        default:
            _errorState->addTransition(this, SIGNAL(retry()), _previousState);
            if (_cfg->config.autoretry)
                _retry.start();
            break;
    }
}

void EceSetup::_onError (const libencloud::Error &err)
{
    LIBENCLOUD_DBG(err.toString());

    emit error((_error = err));
}

void EceSetup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    if (_cfg->config.autoretry)
        emit retry();
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
    connect(_errorState, SIGNAL(entered()), this, SLOT(_onErrorState()));

    _initMsg(_retrInfoMsg);
    connect(&_retrInfoMsg, SIGNAL(error(libencloud::Error)), this, SLOT(_onError(libencloud::Error)));
    connect(&_retrInfoMsg, SIGNAL(need(QString, QVariant)), this, SIGNAL(need(QString, QVariant)));
    // only SECE needs license, which is received via setup module
#ifdef LIBENCLOUD_MODE_SECE
    connect(this, SIGNAL(licenseForward(QUuid)), &_retrInfoMsg, SLOT(licenseReceived(QUuid)));
#endif
    connect(_retrInfoState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_retrInfoState, SIGNAL(entered()), &_retrInfoMsg, SLOT(process()));
    connect(_retrInfoState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrInfoState->addTransition(&_retrInfoMsg, SIGNAL(error(libencloud::Error)), _errorState);
    _retrInfoState->addTransition(&_retrInfoMsg, SIGNAL(processed()), _retrCertState);

    _initMsg(_retrCertMsg);
    connect(&_retrCertMsg, SIGNAL(error(libencloud::Error)), this, SLOT(_onError(libencloud::Error)));
    connect(_retrCertState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_retrCertState, SIGNAL(entered()), &_retrCertMsg, SLOT(process()));
    connect(_retrCertState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrCertState->addTransition(&_retrCertMsg, SIGNAL(error(libencloud::Error)), _errorState);
    _retrCertState->addTransition(&_retrCertMsg, SIGNAL(processed()), _retrConfState);

    _initMsg(_retrConfMsg);
    connect(&_retrConfMsg, SIGNAL(error(libencloud::Error)), this, SLOT(_onError(libencloud::Error)));
    connect(_retrConfState, SIGNAL(entered()), this, SLOT(_stateEntered()));
    connect(_retrConfState, SIGNAL(entered()), &_retrConfMsg, SLOT(process()));
    connect(_retrConfState, SIGNAL(exited()), this, SLOT(_stateExited()));
    _retrConfState->addTransition(&_retrConfMsg, SIGNAL(error(libencloud::Error)), _errorState);
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
