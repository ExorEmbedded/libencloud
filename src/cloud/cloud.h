#ifndef _LIBENCLOUD_PRIV_CLOUD_H_
#define _LIBENCLOUD_PRIV_CLOUD_H_

#include <QObject>
#include <QString>
#include <encloud/Auth>
#include <encloud/Error>
#include <encloud/State>
#include <encloud/Progress>
#include <common/config.h>
#include <common/vpn/vpnclient.h>
#include <common/vpn/vpnmanager.h>

namespace libencloud {

class Cloud : public QObject
{
    Q_OBJECT

public:
    Cloud (Config *cfg);
    ~Cloud ();

    int start ();
    int stop ();

    int getTotalSteps() const;

signals:
    void error (const libencloud::Error &error);
    void stateChanged (State state);
    void progress (const Progress &progress);
    void ipAssigned (const QString &ip);
    void need (const QString &what);
    void authRequired (Auth::Id id);
    void authSupplied (const Auth &auth);

private slots:
    void _vpnStateChanged (VpnClient::State state);
    void _vpnClientErr (VpnClient::Error err, const QString &errMsg);
    void _vpnManagerErr (VpnManager::Error err, const QString &errMsg);
    void _onRetry ();

private:
    Config *_cfg;
    VpnClient *_vpnClient;
    VpnManager *_vpnManager;
    Retry _retry;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CLOUD_H_ */
