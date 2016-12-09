#ifndef _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_

#include <qyamldocument.h>
#include <qyamlmapping.h>
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
    void authRequired (Auth::Id id, const QVariant &params);
    void authChanged (const Auth &auth);
    void processed ();

public slots:
    int process ();
    void authSupplied (const Auth &auth);

private slots:
    void _error (const libencloud::Error &error);
    void _clientComplete (const QString &response, const QMap<QByteArray, QByteArray> &headers);

private:
    // message handling
    int _packRequest ();
    int _encodeRequest (QUrl &url, QUrl &params);
    int _decodeResponse (const QString &response, const QMap<QByteArray, QByteArray> &headers);
    int _unpackResponse ();

    // internals
    int _init ();
    QString _getCode ();
    QByteArray _getKey ();
    QString _calcRegPath ();
    QByteArray _decrypt (const QByteArray &enc);
    int _decodeConfig (const QByteArray &enc);
    int _decodeConfigVpn (const QtYAML::Mapping &mapping);
    int _decodeConfigFallbackVpn (const QtYAML::Mapping &mapping);

    // request inputs
    Auth _sbAuth;

    // response outputs
    VpnConfig _vpnConfig;
    VpnConfig _fallbackVpnConfig;
    QSslCertificate _caCert;

    // internals
    libencloud_crypto_t ec;
    QString _code;
    QByteArray _key;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QCC_REGMSG_H_ */
