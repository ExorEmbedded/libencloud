#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_H_

#include <QStateMachine>
#include <QtPlugin>
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
    EceSetup ();
    int init ();
    int start ();

signals:
    void stateChanged (const QString &state);
    void retry ();

private slots:
    void _stateEntered ();
    void _stateExited ();
    void _onError ();
    void _onRetryTimeout ();

private:
    int initMsg (MessageInterface &msg);

    int retrSbInfo ();

    QStateMachine _fsm;
    Client _client;

    QState *_initialState;
    QState *_previousState;
    QState _errorState;
    bool _error;
    int _backoff;

    RetrInfoMsg _retrInfoMsg;  // initial state
    QState _retrInfoState;

    RetrCertMsg _retrCertMsg;
    QState _retrCertState;

    RetrConfMsg _retrConfMsg;
    QState _retrConfState;

    QState _checkExpiryState;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_H_ */
