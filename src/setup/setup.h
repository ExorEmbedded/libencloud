#ifndef _LIBENCLOUD_PRIV_SETUP_H_
#define _LIBENCLOUD_PRIV_SETUP_H_

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <cloud/vpnconfig.h>

namespace libencloud {

class SetupInterface
{
public:
    virtual ~SetupInterface () {};

    void setConfig (Config *cfg);
    virtual int init () = 0;

    virtual int start () = 0;
    virtual const VpnConfig *getVpnConfig () = 0;

signals:
    virtual void stateChanged (const QString &state) = 0;
    virtual void completed () = 0;

protected:
    Config *_cfg;
};

} // namespace libencloud

Q_DECLARE_INTERFACE(libencloud::SetupInterface, "com.endian.libencloud.SetupInterface/0.1")

#endif  /* _LIBENCLOUD_PRIV_SETUP_H_ */
