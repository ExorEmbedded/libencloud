#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_H_

#include <QStateMachine>
#include <QtPlugin>
#include <QUuid>
#include <common/message.h>
#include <setup/setup.h>
#include <setup/ece/retrinfomsg.h>
#include <setup/ece/retrcertmsg.h>
#include <setup/ece/retrconfmsg.h>

// initial timeout for retry (in seconds) - grows exponentially
#define LIBENCLOUD_RETRY_TIMEOUT    3

namespace libencloud {

class EceSetup : public QObject, public SetupInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::SetupInterface)

public:
    EceSetup (Config *cfg);
    int start ();
    int stop ();

    const VpnConfig *getVpnConfig ();

signals:

    void stateChanged (const QString &state);
    void retry ();
    void completed ();

#ifdef LIBENCLOUD_MODE_SECE
    void need (const QString &what);

    void licenseForward (QUuid uuid);
#endif

private slots:
    void _stateEntered ();
    void _stateExited ();
    void _onError ();
    void _onRetryTimeout ();

private:
    int _initFsm ();
    int _initMsg (MessageInterface &msg);
    QString _stateStr (QState *state);

    QStateMachine _fsm;
    Client _client;

    QState *_initialState;
    QState *_completedState;
    QState *_previousState;

    QState _errorSt, *_errorState;

    RetrInfoMsg _retrInfoMsg;
    QState _retrInfoSt, *_retrInfoState;

    RetrCertMsg _retrCertMsg;
    QState _retrCertSt, *_retrCertState;

    RetrConfMsg _retrConfMsg;
    QState _retrConfSt, *_retrConfState;

    QState _checkExpirySt, *_checkExpiryState;

    bool _error;
    int _backoff;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_H_ */
