#ifndef _LIBENCLOUD_PRIV_SETUP_H_
#define _LIBENCLOUD_PRIV_SETUP_H_

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <common/vpn/vpnconfig.h>

namespace libencloud {

class SetupInterface
{
public:
    SetupInterface (Config *cfg);
    virtual ~SetupInterface () {};

    virtual int start () = 0;
    virtual int stop () = 0;
    virtual const VpnConfig *getVpnConfig () = 0;

signals:
    virtual void stateChanged (const QString &state) = 0;
    virtual void need (const QString &what) = 0;
    virtual void completed () = 0;

protected:
    Config *_cfg;
};

} // namespace libencloud

Q_DECLARE_INTERFACE(libencloud::SetupInterface, "com.endian.libencloud.SetupInterface/0.1")

#endif  /* _LIBENCLOUD_PRIV_SETUP_H_ */
