#ifndef _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_

#include <yaml-cpp/yaml.h>
#include <QString>
#include <QVariant>
#include <encloud/Auth>
#include <encloud/Client>
#include <encloud/Vpn/VpnConfig>
#include <common/message.h>

#define LIBENCLOUD_SETUP_QCC_REG_URL        "/manage/access/registry"

namespace libencloud {

class RegMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

public:
    RegMsg();
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
    void _gotRedirect (const QString &response, const QMap<QByteArray, QByteArray> &headers);
    void _processRedirect ();
    void _gotConfig (const QString &response, const QMap<QByteArray, QByteArray> &headers);
    void _completeSetup ();

private:
    // message handling
    int _decodeRedirect (const QString &response);
    int _decodeConfig (const QString &response);
    int _unpackConfig ();

    // internals
    int _init ();
    QByteArray _getCode ();
    QByteArray _hash (const QByteArray &in);
    QByteArray _calcRegPath ();
    QByteArray _calcConfigPath ();
    QByteArray _decrypt (const QByteArray &key, const QByteArray &enc);
    QString _yamlNodeToStr (const YAML::Node *nodeP);
    int _yamlNodeToInt (const YAML::Node *nodeP);
    int _decodeRedirect (const QByteArray &enc);
    int _decodeConfig (const QByteArray &enc);
    int _decodeConfigVpn (const YAML::Node &node);
    int _decodeConfigFallbackVpn (const YAML::Node &node);

    // request inputs
    Auth _sbAuth;

    // response outputs
    VpnConfig _vpnConfig;
    VpnConfig _fallbackVpnConfig;
    QSslCertificate _caCert;

    // internals
    libencloud_crypto_t ec;
    QByteArray _key;
    QByteArray _provisioningEnc;
    QUrl _redirectUrl;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_ */
