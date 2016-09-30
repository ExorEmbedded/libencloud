#ifndef _LIBENCLOUD_PRIV_SETUP_H_
#define _LIBENCLOUD_PRIV_SETUP_H_

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <QUuid>
#include <encloud/Auth>
#include <encloud/Error>
#include <encloud/Progress>
#include <encloud/State>
#include <encloud/Vpn/VpnConfig>

namespace libencloud {

class SetupInterface : public QObject
{
public:
    SetupInterface (Config *cfg);
    virtual ~SetupInterface () {};

    virtual int start () = 0;
    virtual int stop (bool reset = true, bool close = false) = 0;

    virtual const VpnConfig *getVpnConfig () const = 0;
    virtual const VpnConfig *getFallbackVpnConfig () const = 0;

    virtual int getTotalSteps() const = 0;

signals:
    //
    // setup -> core
    //
    virtual void error (const libencloud::Error &err) = 0;
    virtual void progress (const Progress &progress) = 0;
    virtual void completed () = 0;

    //
    // internal -> setup -> core
    //
    virtual void need (const QString &what, const QVariant &params) = 0;
    virtual void authRequired (Auth::Id id, const QVariant &params) = 0;

    //
    // core -> setup -> internal
    //

    virtual void authSupplied (const Auth &auth) = 0; 

#ifdef LIBENCLOUD_MODE_SECE
    virtual void licenseForward (const QUuid &uuid) = 0;
#endif

protected:
    Config *_cfg;
};

} // namespace libencloud

Q_DECLARE_INTERFACE(libencloud::SetupInterface, "com.endian.libencloud.SetupInterface/0.1")

#endif  /* _LIBENCLOUD_PRIV_SETUP_H_ */
