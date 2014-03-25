#ifndef _LIBENCLOUD_PRIV_SETUP_QIC_H_
#define _LIBENCLOUD_PRIV_SETUP_QIC_H_

#include <QtPlugin>
#include <encloud/Client>
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
        StateSetupMsg = 1,
        StateReceived,

        // used for total count
        StateFirst = StateSetupMsg,
        StateLast = StateReceived
    } State;

    QicSetup (Config *cfg);

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
    void serverConfigSupply (const QVariant &variant);
    void completed ();

    //
    // internal -> setup -> core
    //
    void need (const QString &what);
    void authRequired (Auth::Id id);

    //
    // core -> setup -> internal
    //
    void authSupplied (const Auth &auth);  

private slots:
    void _onProcessed ();

private:
    int _initMsg (MessageInterface &msg);

    SetupMsg _setupMsg;
    Client _client;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QIC_H_ */
