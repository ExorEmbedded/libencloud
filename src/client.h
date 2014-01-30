#ifndef _LIBENCLOUD_PRIV_CLIENT_H_
#define _LIBENCLOUD_PRIV_CLIENT_H_

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QEventLoop>
#include <QSslSocket>
#include <QSslKey>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <encloud/core.h>
#include "common.h"
#include "helpers.h"
#include "defaults.h"
#include "config.h"
#include "msg.h"

namespace libencloud {

class Client : public QObject
{
    Q_OBJECT

public:
    Client();
    ~Client();

    libencloud_rc setConfig (libencloud::Config *cfg);
    libencloud_rc run (libencloud::ProtocolType protocol, libencloud::Message &message);

public slots:
    void timerTimeout ();

private slots:
    void proxyAuthenticationRequiredSlot (const QNetworkProxy &proxy, QAuthenticator *authenticator);
    void sslErrorsSlot (QNetworkReply *reply, const QList<QSslError> &errors);
    void finishedSlot (QNetworkReply *reply);
    void networkErrorSlot (QNetworkReply::NetworkError);
    void timeoutSlot ();

private:
    libencloud_rc __run (const QUrl &url, const QUrl &params, const QSslConfiguration &sslconf, QString &response);
    libencloud_rc __loadSslConf (libencloud::ProtocolType protocol, QUrl &url, QSslConfiguration &sslconf);

    libencloud_rc error;
    Config *cfg;
    QEventLoop *loop;
    QNetworkReply *reply;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CLIENT_H_ */
