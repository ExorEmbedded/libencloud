#ifndef _LIBENCLOUD_PRIV_SETUP_QCC_H_
#define _LIBENCLOUD_PRIV_SETUP_QCC_H_

#include <QStateMachine>
#include <QtPlugin>
#include <encloud/Client>
#include <common/message.h>
#include <setup/setup.h>
#include <setup/qcc/setupmsg.h>
#include <setup/qcc/loginmsg.h>

namespace libencloud {

class QccSetup : public SetupInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::SetupInterface)

public:
    typedef enum {
        StateInvalid = -1,
        StateError = 0,

        // used for steps
        StateSetupMsg = 1,
        StateReceived,

        // used for total count
        StateFirst = StateSetupMsg,
        StateLast = StateReceived
    } State;

    QccSetup (Config *cfg);

    int start ();
    int stop (bool reset = true);

    const VpnConfig *getVpnConfig () const;
    const VpnConfig *getFallbackVpnConfig () const;

    int getTotalSteps() const;

signals:
    //
    // setup -> core
    //
    void error (const libencloud::Error &error);
    void progress (const Progress &progress);
    void serverConfigSupply (const QVariant &variant);
    void completed ();

    //
    // internal -> setup -> core
    //
    void need (const QString &what, const QVariant &params);
    void authRequired (Auth::Id id, const QVariant &params);

    //
    // core -> setup -> internal
    //
    void authSupplied (const Auth &auth);  

    //
    // internals
    // 
    void retry ();

private slots:
    void _stateEntered ();
    void _stateExited ();
    void _onProcessed ();
    void _onErrorState ();
    void _onError (const libencloud::Error &error);
    void _onRetryTimeout ();

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

    SetupMsg _setupMsg;
    QState _setupMsgSt, *_setupMsgState;
    QState _finalSt, *_finalState;

    LoginMsg _loginMsg;

    bool _isError;
    Error _error;
    Retry _retry;

    VpnConfig _vpnConfig;
    VpnConfig _vpnFallbackConfig;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QCC_H_ */
