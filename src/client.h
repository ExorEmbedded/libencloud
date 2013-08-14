#ifndef _ECE_CLIENT_H_
#define _ECE_CLIENT_H_

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
#include <ece.h>
#include "common.h"
#include "helpers.h"
#include "defaults.h"
#include "config.h"
#include "msg.h"

namespace Ece {

class Client : public QObject
{
    Q_OBJECT

public:
    Client();
    ~Client();

    ece_rc_t setConfig (Ece::Config *cfg);
    ece_rc_t run (Ece::ProtocolType protocol, Ece::Message &message);

private slots:
    void proxyAuthenticationRequiredSlot (const QNetworkProxy &proxy, QAuthenticator *authenticator);
    void sslErrorsSlot (QNetworkReply *reply, const QList<QSslError> &errors);
    void finishedSlot (QNetworkReply *reply);
    void networkErrorSlot (QNetworkReply::NetworkError);
    void timeoutSlot ();

private:
    ece_rc_t __run (const QUrl &url, const QUrl &params, const QSslConfiguration &sslconf, QString &response);
    ece_rc_t __loadSslConf (Ece::ProtocolType protocol, QUrl &url, QSslConfiguration &sslconf);

    ece_rc_t error;
    Config *cfg;
    QEventLoop *loop;
    QNetworkReply *reply;
};

} // namespace Ece

#endif
