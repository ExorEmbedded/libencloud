#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_H_

#include <QStateMachine>
#include <QtPlugin>
#include <QUuid>
#include <encloud/Client>
#include <common/message.h>
#include <setup/setup.h>
#include <setup/ece/retrinfomsg.h>
#include <setup/ece/retrcertmsg.h>
#include <setup/ece/retrconfmsg.h>

namespace libencloud {

class EceSetup : public QObject, public SetupInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::SetupInterface)

    typedef enum {
        StateInvalid = -1,
        StateError = 0,

        // used for steps
        StateInit = 1,
        StateRetrInfo,
        StateRetrCert,
        StateRetrConf,
        StateCheckExpiry,

        // used for total count
        StateFirst = StateInit,
        StateLast = StateCheckExpiry
    } State;

public:
    EceSetup (Config *cfg);

    int start ();
    int stop ();

    const VpnConfig *getVpnConfig ();

    int getTotalSteps() const;

signals:
    //
    // setup -> core
    //
    void error (QString msg = "");
    void progress (const Progress &progress);
    void completed ();

    //
    // internal -> setup -> core
    //
    void need (const QString &what);
    void authRequired (Auth::Id id);

    //
    // core -> setup -> internal
    //
#ifdef LIBENCLOUD_MODE_SECE
    void licenseForward (const QUuid &uuid);
#endif
   
    //
    // internals
    // 
    void retry ();

private slots:
    void _stateEntered ();
    void _stateExited ();
    void _onError (QString msg = "");
    void _onRetryTimeout ();

private:
    int _initFsm ();
    int _initMsg (MessageInterface &msg);
    Progress _stateToProgress (QState *state);

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
