#ifndef _LIBENCLOUD_PRIV_SETUP_REG_H_
#define _LIBENCLOUD_PRIV_SETUP_REG_H_

#include <QStateMachine>
#include <QtPlugin>
#include <encloud/Client>
#include <common/message.h>
#include <setup/setup.h>
#include <setup/cc2/cc2msg.h>
#include <setup/qcc/qccsetup.h>
#include <setup/common/closemsg.h>

namespace libencloud {

class CC2Setup : public SetupInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::SetupInterface)

public:
    typedef enum {
        StateInvalid = -1,
        StateError = 0,

        // used for steps
        StateCC2Msg = 1,
        StateQCCStarted,
        StateQCCCompleted,

        // used for total count
        StateFirst = StateCC2Msg,
        StateLast = StateQCCCompleted
    } State;

    CC2Setup (Config *cfg);

    int start ();
    int stop (bool reset, bool close);

    const VpnConfig *getVpnConfig () const;
    const VpnConfig *getFallbackVpnConfig () const;

    int getTotalSteps() const;

signals:
    //
    // core -> setup -> internal
    //
    void authSupplied (const libencloud::Auth &auth);  

    //
    // setup -> core
    //
    void error (const libencloud::Error &err);
    void progress (const Progress &progress);
    void serverConfigSupply (const QVariant &variant);
    void completed ();
    void stopped ();

    //
    // internal -> setup -> core
    //
    void need (const QString &what, const QVariant &params);
    void authRequired (libencloud::Auth::Id id, const QVariant &params);
    void authChanged (const libencloud::Auth &auth);  

    //
    // internals
    // 
    void retry ();
    void retryAfterConsecutiveErrorsInQccSetup();

private slots:
    void _stateEntered ();
    void _stateExited ();
    void _onProcessed ();
    void _onCloseProcessed ();
    void _onErrorState ();
    void _onError (const libencloud::Error &error);
    void _onRetryTimeout ();
    void _onQccError(const libencloud::Error &error);

private:
    int _initFsm ();
    int _deinitFsm (bool reset = true);
    int _initMsg (MessageInterface &msg);
    void _clear ();
    Progress _stateToProgress (QState *state);

    QStateMachine _fsm;
    bool m_setupEnabled;

    QState *_initialState;
    QState *_previousState;

    QState _errorSt, *_errorState;
    
    QccSetup _qccSetup;
    QState _qccSetupSt, *_qccSetupState;

    CC2Msg _cc2Msg;
    QState _cc2MsgSt, *_cc2MsgState;
    QState _finalSt, *_finalState;

    CloseMsg _closeMsg;

    bool _isError;
    Error _error;
    Retry _retry;
    int _qccConsecutiveErrors;

    VpnConfig _vpnConfig;
    VpnConfig _vpnFallbackConfig;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_REG_H_ */
