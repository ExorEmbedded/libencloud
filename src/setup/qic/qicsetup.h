#ifndef _LIBENCLOUD_PRIV_SETUP_QIC_H_
#define _LIBENCLOUD_PRIV_SETUP_QIC_H_

#include <QtPlugin>
#include <common/client.h>
#include <common/message.h>
#include <setup/setup.h>
#include <setup/qic/setupmsg.h>

namespace libencloud {

class QicSetup : public QObject, public SetupInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::SetupInterface)

public:
    typedef enum {
        StateInvalid = -1,
        StateError = 0,

        // used for steps
        StateInit = 1,
        StateSetupMsg,

        // used for total count
        StateFirst = StateInit,
        StateLast = StateSetupMsg
    } State;

    QicSetup (Config *cfg);

    int start ();
    int stop ();

    const VpnConfig *getVpnConfig ();

    int getTotalSteps() const;

signals:
    // this -> other
    void error (QString msg = "");
    void progress (const Progress &progress);
    void need (const QString &what);
    void authRequired (const QString &type);
    void completed ();

    // other -> this
    void authSupplied (const Auth &auth);  

private slots:
    void _onError (QString msg = "");
    void _onProcessed ();
    void _onRetryTimeout ();

private:
    int _initMsg (MessageInterface &msg);

    SetupMsg _setupMsg;
    Client _client;

    int _backoff;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QIC_H_ */
