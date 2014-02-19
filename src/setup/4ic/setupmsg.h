#ifndef _LIBENCLOUD_PRIV_SETUP_4IC_SETUPMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_4IC_SETUPMSG_H_

#include <QString>
#include <encloud/Auth>
#include <common/client.h>
#include <common/message.h>
#include <common/vpn/vpnconfig.h>

#define LIBENCLOUD_SETUP_4IC_CONFIG_URL        "/manage/status/status.access.config/"

namespace libencloud {

class SetupMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

public:
    const VpnConfig *getVpnConfig ();

signals:
    void error (QString msg = "");
    void need (const QString &what);
    void processed ();

public slots:
    void process ();
    void authSupplied (const Auth &auth);

private slots:
    void _clientComplete (const QString &response);

private:
    int _packRequest ();
    int _encodeRequest (QUrl &url, QUrl &params);
    int _decodeResponse (const QString &response);
    int _unpackResponse ();

    //request inputs
    Auth _auth;

    //response outputs
    VpnConfig _vpnConfig;
    QSslCertificate _caCert;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_4IC_SETUPMSG_H_ */
