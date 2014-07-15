#ifndef _LIBENCLOUD_PRIV_SETUP_QCC_LOGINMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_QCC_LOGINMSG_H_

#include <QString>
#include <QVariant>
#include <encloud/Auth>
#include <encloud/Client>
#include <common/message.h>
#include <common/vpn/vpnconfig.h>

#define LIBENCLOUD_SETUP_QCC_LOGIN_URL        "/manage/access/login/"
#define LIBENCLOUD_SETUP_QCC_LOGOUT_URL        "/manage/access/logout/"

namespace libencloud {

class LoginMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

signals:
    void error (const libencloud::Error &err);
    void authRequired (Auth::Id id);
    void processed ();

public slots:
    int process ();
    void authSupplied (const Auth &auth);

private slots:
    void _clientComplete (const QString &response);

private:
    int _packRequest ();
    int _encodeRequest (QUrl &url, QUrl &params);
    int _decodeResponse (const QString &response);
    int _unpackResponse ();

    //request inputs
    Auth _sbAuth;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QCC_LOGINMSG_H_ */
