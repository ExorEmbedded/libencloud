#ifndef _LIBENCLOUD_PRIV_SETUP_H_
#define _LIBENCLOUD_PRIV_SETUP_H_

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <QUuid>
#include <encloud/Auth>
#include <encloud/Progress>
#include <encloud/State>
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

    virtual int getTotalSteps() const = 0;

signals:
    // this -> other
    virtual void error (QString msg = "") = 0;
    virtual void progress (const Progress &progress) = 0;
    virtual void need (const QString &what) = 0;
    virtual void completed () = 0;

    // other -> this (optional overrides)
    void authSupplied (const Auth &auth);  
#ifdef LIBENCLOUD_MODE_SECE
    virtual void licenseForward (QUuid uuid) = 0;
#endif

protected:
    Config *_cfg;
};

} // namespace libencloud

Q_DECLARE_INTERFACE(libencloud::SetupInterface, "com.endian.libencloud.SetupInterface/0.1")

#endif  /* _LIBENCLOUD_PRIV_SETUP_H_ */
