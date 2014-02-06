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
    void completed ();

private slots:
    void _stateEntered ();
    void _stateExited ();
    void _onError ();
    void _onRetryTimeout ();

private:
    int _initMsg (MessageInterface &msg);
    QString _stateStr (QState *state);

    int retrSbInfo ();

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
