#ifndef _ENCLOUD_PRIV_CLIENT_H_
#define _ENCLOUD_PRIV_CLIENT_H_

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

namespace encloud {

class Client : public QObject
{
    Q_OBJECT

public:
    Client();
    ~Client();

    encloud_rc setConfig (encloud::Config *cfg);
    encloud_rc run (encloud::ProtocolType protocol, encloud::Message &message);

private slots:
    void proxyAuthenticationRequiredSlot (const QNetworkProxy &proxy, QAuthenticator *authenticator);
    void sslErrorsSlot (QNetworkReply *reply, const QList<QSslError> &errors);
    void finishedSlot (QNetworkReply *reply);
    void networkErrorSlot (QNetworkReply::NetworkError);
    void timeoutSlot ();

private:
    encloud_rc __run (const QUrl &url, const QUrl &params, const QSslConfiguration &sslconf, QString &response);
    encloud_rc __loadSslConf (encloud::ProtocolType protocol, QUrl &url, QSslConfiguration &sslconf);

    encloud_rc error;
    Config *cfg;
    QEventLoop *loop;
    QNetworkReply *reply;
};

} // namespace encloud

#endif  /* _ENCLOUD_PRIV_CLIENT_H_ */
