#ifndef _LIBENCLOUD_PRIV_SETUP_QCC_SETUPMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_QCC_SETUPMSG_H_

#include <QString>
#include <QVariant>
#include <encloud/Auth>
#include <encloud/Client>
#include <encloud/Vpn/VpnConfig>
#include <common/message.h>

#define LIBENCLOUD_SETUP_QCC_CONFIG_URL        "/manage/status/status.access.config/"

namespace libencloud {

class SetupMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

public:
    SetupMsg();
    int clear ();
    const VpnConfig *getVpnConfig () const;
    const VpnConfig *getFallbackVpnConfig () const;

signals:
    void error (const libencloud::Error &err);
    void need (const QString &what);
    void authRequired (Auth::Id id);
    void serverConfigSupply (const QVariant &variant);
    void processed ();

public slots:
    int process ();
    void authSupplied (const Auth &auth);

private slots:
    void _error (const libencloud::Error &error);
    void _clientComplete (const QString &response);

private:
    int _packRequest ();
    int _encodeRequest (QUrl &url, QUrl &params);
    int _decodeResponse (const QString &response);
    int _unpackResponse ();

    //request inputs
    Auth _sbAuth;

    //response outputs
    VpnConfig _vpnConfig;
    VpnConfig _fallbackVpnConfig;
    QSslCertificate _caCert;
    QVariantMap _serverConfig;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QCC_SETUPMSG_H_ */
