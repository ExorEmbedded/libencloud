#ifndef _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_

#include <QString>
#include <QVariant>
#include <encloud/Auth>
#include <encloud/Client>
#include <encloud/Vpn/VpnConfig>
#include <common/message.h>

#define LIBENCLOUD_SETUP_QCC_REG_URL        "/manage/access/registry"

namespace libencloud {

class CC2Msg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

public:
    CC2Msg();
    ~CC2Msg();
    int clear ();
    const VpnConfig *getVpnConfig () const;
    const VpnConfig *getFallbackVpnConfig () const;

signals:
    void error (const libencloud::Error &err);
    void authRequired (libencloud::Auth::Id id, const QVariant &params);
    void authChanged (const libencloud::Auth &auth);
    void processed ();

public slots:
    int process ();
    void authSupplied (const libencloud::Auth &auth);

private slots:
    void _error (const libencloud::Error &error);
    void _gotConfig (const QString &response, const QMap<QByteArray, QByteArray> &headers);
    void _completeSetup ();

private:
    // message handling
    int _decodeConfig (const QString &response);
    int _unpackConfig (const QByteArray &provisioning);

    // internals
    int _init ();
    QByteArray _getCode ();
    QByteArray _calcRegPath ();
    QByteArray _calcConfigPath ();

    // request inputs
    Auth _sbAuth;
    Auth _retrievedSbAuth;

    // internals
    libencloud_crypto_t _ec;
    QByteArray _key;
    QByteArray _provisioning;
    QUrl _regUrl;
    QUrl _redirectUrl;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_ */
