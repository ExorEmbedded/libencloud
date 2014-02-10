#ifndef _LIBENCLOUD_PRIV_VPNMANAGER_H_
#define _LIBENCLOUD_PRIV_VPNMANAGER_H_

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTimer>
#include <common/vpn/vpnconfig.h>
#include <common/vpn/vpnclient.h>

#define LIBENCLOUD_VPNMANAGER_STATE_PERIOD    1000  // (ms)
#define LIBENCLOUD_VPNMANAGER_ATTACH_RETRIES  5

namespace libencloud {

class VpnManager : public QObject
{
    Q_OBJECT

public:
    // keep me aligned with Manager::errorString
    typedef enum
    {
        NoError = 0,
        MemoryError,
        CommError,
        MgmtError,
        UnhandledError,
        SocketError,
        AuthError
    }
    Error;

    typedef enum
    {
        StateDetached = 0,
        StateAttaching,
        StateAttached,
        StateDetaching
    }
    State;

    VpnManager (const Config *config);
    ~VpnManager ();

    Error error ();
    static QString errorString (Error err);
    
    State state ();
    static QString stateString (State st);

    void attach (VpnClient *client, QString host, int port);
    void detach ();

signals:
    void authRequest ();
    void proxyAuthRequest ();
    void sigError (VpnManager::Error err, QString msg = "");

public slots:
    void authSupply (QString user, QString pass);
    void proxyAuthSupply (QString user, QString pass);

private slots:
    void retryAttach ();
    void stateTimeout ();
    void hostFound ();
    void connected ();
    void disconnected ();
    void readyRead ();
    void readChannelFinished ();
    void bytesWritten (qint64 bytes);
    void socketError (QAbstractSocket::SocketError err);

private:
    void parseLine (QByteArray line);
    void parseLinePass (QByteArray rest);
    void parseLineState (QByteArray line);
    void sendAuth (const QString type, const QString &user, const QString &pass);

    const Config *config;
    Error err;
    State st;
    QTcpSocket *socket;
    int attachRetries;

    VpnClient *client;
    QString host;
    int port;

    QTimer *stateTimer;
};

}  // namespace libencloud

#endif  /* _LIBENCLOUD_VPNMANAGER_H_ */

