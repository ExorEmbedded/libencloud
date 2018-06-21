#ifndef _LIBENCLOUD_PRIV_SETUP_VPN_H_
#define _LIBENCLOUD_PRIV_SETUP_VPN_H_

#include <QtPlugin>
#include <encloud/Client>
#include <common/message.h>
#include <setup/setup.h>

namespace libencloud {

class VpnSetup : public SetupInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::SetupInterface)

public:
    typedef enum {
        StateInvalid = -1,
        StateError = 0,

        // used for total count
        StateFirst = 1,
        StateLast = StateFirst
    } State;

    VpnSetup (Config *cfg);

    int start ();
    int stop ();

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
    void authRequired (libencloud::Auth::Id id);

    //
    // core -> setup -> internal
    //
    void authSupplied (const libencloud::Auth &auth);


private:
    void _clear ();
    VpnConfig _vpnConfig;
    VpnConfig _vpnFallbackConfig;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QCC_H_ */
