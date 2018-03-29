#define LIBENCLOUD_DISABLE_TRACE
#include <QTcpSocket>
#include <QStringList>
#include <QRegExp>
#include <encloud/Http/HttpServer>
#include <encloud/Http/HttpRequest>
#include <encloud/Http/HttpResponse>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

HttpServer::HttpServer (QObject *parent, Server *server)
    : QTcpServer(parent)
    , _server(server)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (server == NULL);
err:
    return;
}

HttpServer::~HttpServer ()
{
    LIBENCLOUD_TRACE;

    close();
}

int HttpServer::setHandler (HttpAbstractHandler *handler)
{
    LIBENCLOUD_ERR_IF (handler == NULL);

    _handler = handler;

    return 0;
err:
    return ~0;
}

int HttpServer::start (const QHostAddress &address, quint16 port)
{
    enum { MAX_ATTEMPTS = 10 };
    int i;

    if (isListening())
        return 0;

    // no setting - scan for a free port
    for (i = 0; i < MAX_ATTEMPTS && !listen(address, port); i++, port++) 
        ;
    LIBENCLOUD_ERR_MSG_IF (i == MAX_ATTEMPTS,
            "Could not bind to any port - scanned " <<
            QString::number(MAX_ATTEMPTS) << " ports from " << QString::number(port - MAX_ATTEMPTS));

    LIBENCLOUD_DBG("[HttpServer] bound to port: " << QString::number(port));

    emit portBound(port);

    return 0;
err:
    return ~0;
}

int HttpServer::stop ()
{
    LIBENCLOUD_TRACE;
    
    if (isListening())
        close();

    return 0;
}

void HttpServer::incomingConnection (int sd)
{
    LIBENCLOUD_TRACE;

    QTcpSocket *socket = new QTcpSocket(this); 
    LIBENCLOUD_ERR_IF (socket == NULL);

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), 
            this, SLOT(error(QAbstractSocket::SocketError)));
    socket->setSocketDescriptor(sd);

err:
    return;
}

//
// private slots
//

void HttpServer::readyRead ()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    QByteArray inData;
    QByteArray outData;

//    LIBENCLOUD_DBG("bytesAvailable: " << QString::number(socket->bytesAvailable()));

    inData = socket->readAll();
    LIBENCLOUD_ERR_IF (inData.isEmpty());

//    LIBENCLOUD_DBG("<<<" << QString(inData).replace(LIBENCLOUD_HTTP_NL, " | "));

    // if nothing has been received yet we need a header
    if (_contexts[socket].data.isEmpty())
    {
        HttpHeaders headers;
        LIBENCLOUD_ERR_IF (headers.decode(inData));

//        LIBENCLOUD_DBG("header size: " << headers.getSize() << 
//                ", content-length: " << headers.get("content-length"));
        
        _contexts[socket].bytesExpected = headers.getSize() + headers.get("content-Length").toInt();
    }

    _contexts[socket].data.append(inData);

    // check if message is complete
    if (_contexts[socket].data.size() >= _contexts[socket].bytesExpected)
    {
        outData = handleMessage(_contexts[socket].data);
        LIBENCLOUD_ERR_IF (outData.isEmpty());

//        LIBENCLOUD_DBG(">>>" << QString(outData).replace(LIBENCLOUD_HTTP_NL, " | "));

        socket->write(outData);
        _contexts[socket].data.clear();
    }

err:
    return;
}

void HttpServer::disconnected ()
{
    LIBENCLOUD_TRACE;

    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
    _contexts.remove(socket);
}

void HttpServer::error (QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    QTcpSocket *socket = (QTcpSocket*) sender();

    switch (socket->error())
    {
        // silent errors
        case QAbstractSocket::RemoteHostClosedError:
            break;
        default:
            LIBENCLOUD_DBG("[HttpServer] err: " << QString::number(socket->error())
                    << " (" << socket->errorString() << ")");
    }

    socket->deleteLater();
    _contexts.remove(socket);
}

//
// private methods
//

QByteArray HttpServer::handleMessage (QByteArray message)
{
    LIBENCLOUD_TRACE;

    QByteArray outMessage;
    HttpRequest request;
    HttpResponse response;

    LIBENCLOUD_ERR_IF (_handler == NULL);
    LIBENCLOUD_ERR_IF (request.decode(message));
    _handler->handle(request, response);
    LIBENCLOUD_ERR_IF (response.encode(outMessage));

    return outMessage;
err:
    return "";
}

}  // namespace libencloud
