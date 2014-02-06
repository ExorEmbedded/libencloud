#ifndef _LIBENCLOUD_PRIV_CLIENT_H_
#define _LIBENCLOUD_PRIV_CLIENT_H_

#include <QObject>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QAuthenticator>
#include <QSslConfiguration>

namespace libencloud {

class Client : public QObject
{
    Q_OBJECT

public:
    Client ();
    void run (const QUrl &url, const QUrl &params, const QMap<QByteArray, QByteArray> &headers, 
            const QSslConfiguration &conf);

signals:
    void error ();
    void complete (const QString &reponse);

protected slots:
    void _proxyAuthenticationRequired (const QNetworkProxy &proxy, QAuthenticator *authenticator);
    void _sslErrors (QNetworkReply *reply, const QList<QSslError> &errors);
    void _networkError (QNetworkReply::NetworkError err);
    void _finished (QNetworkReply *reply);

protected:
    QNetworkAccessManager _qnam;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CLIENT_H_ */
