#ifndef _LIBENCLOUD_PRIV_CLOUD_H_
#define _LIBENCLOUD_PRIV_CLOUD_H_

#include <QObject>
#include <QString>
#include <encloud/Auth>
#include <encloud/Error>
#include <encloud/State>
#include <encloud/Progress>
#include <encloud/Vpn/VpnClient>
#include <encloud/Vpn/VpnManager>
#include <common/config.h>
#include <setup/setup.h>

namespace libencloud {

class Cloud : public QObject
{
    Q_OBJECT

public:
    Cloud (Config *cfg);
    ~Cloud ();

    inline void setSetup (SetupInterface *setup)        { _setup = setup; };

    int start (bool fallback = false);
    int stop ();

    int getTotalSteps() const;

signals:
    void error (const libencloud::Error &error);
    void stateChanged (State state);
    void progress (const Progress &progress);
    void ipAssigned (const QString &ip);
    void fallback (bool isFallback);
    void need (const QString &what, const QVariant &params);
    void authRequired (Auth::Id id);
    void authSupplied (const Auth &auth);

private slots:
    void _vpnStateChanged (VpnClient::State state);
    void _vpnClientErr (VpnClient::Error err, const QString &errMsg);
    void _vpnManagerErr (VpnManager::Error err, const QString &errMsg);
    void _onRetry ();

private:
    void _restart (bool fallback = false, bool force = false);

    Config *_cfg;
    SetupInterface *_setup;
    bool _isFallback;
    VpnClient *_vpnClient;
    VpnManager *_vpnManager;
    Retry _retry;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CLOUD_H_ */
