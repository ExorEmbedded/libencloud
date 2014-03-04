#ifndef _LIBENCLOUD_PRIV_HTTP_SERVER_H_
#define _LIBENCLOUD_PRIV_HTTP_SERVER_H_

#include <QTcpServer>
#include <encloud/HttpAbstractHandler>
#include <encloud/HttpServer>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

typedef struct
{
    QByteArray data;
    qint64 bytesExpected;
} 
http_context_t;

class HttpServerPriv : public QTcpServer
{
    Q_OBJECT

public:
    HttpServerPriv (HttpServer *server);
    ~HttpServerPriv ();

    int setHandler (HttpAbstractHandler *handler);

    int start (const QHostAddress &address = QHostAddress::Any, 
            quint16 port = 0);
    int stop ();

protected:
    void incomingConnection (int sd);

private slots:
    void readyRead ();
    void disconnected ();
    void error (QAbstractSocket::SocketError socketError);

private:
    QByteArray handleMessage (QByteArray message);
    QVariant handleJson (QVariant json);

    HttpServer *_server;
    QMap<QTcpSocket *, http_context_t> _contexts;
    HttpAbstractHandler *_handler;
};

}  // namespace libencloud

#endif  // _LIBENCLOUD_PRIV_HTTP_SERVER_H_
