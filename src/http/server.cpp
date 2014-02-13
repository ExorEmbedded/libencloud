#include <QTcpSocket>
#include <QStringList>
#include <QRegExp>
#include <encloud/HttpRequest>
#include <encloud/HttpServer>
#include <common/common.h>
#include <common/config.h>

#define LIBENCLOUD_SRV_LISTEN          "127.0.0.1"

//#ifdef LIBENCLOUD_DISABLE_SERVICE
#define LIBENCLOUD_SVC_TRACE         LIBENCLOUD_TRACE
#define LIBENCLOUD_SVC_LOG(msg)      LIBENCLOUD_DBG(msg)
#define LIBENCLOUD_SVC_ERR_IF(cond)  LIBENCLOUD_ERR_IF(cond)
#define LIBENCLOUD_SVC_RETURN_IF(cond, res) LIBENCLOUD_RETURN_IF(cond, res)
#define LIBENCLOUD_SVC_DBG(msg)      LIBENCLOUD_DBG(msg)
//#else
//#  include "service.h"
//#endif

// disable heavy tracing
#undef LIBENCLOUD_SVC_TRACE 
#define LIBENCLOUD_SVC_TRACE do {} while(0)

namespace libencloud 
{

//
// public methods
//

HttpServer::HttpServer (QObject *parent)
    : QTcpServer(parent)
{
    LIBENCLOUD_SVC_TRACE;
}

HttpServer::~HttpServer ()
{
    LIBENCLOUD_SVC_TRACE;

    LIBENCLOUD_DELETE(_socket);
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

int HttpServer::start ()
{
    LIBENCLOUD_SVC_TRACE;

    LIBENCLOUD_ERR_IF(!listen(QHostAddress(LIBENCLOUD_SRV_LISTEN), 
                LIBENCLOUD_SRV_PORT_DFT));  
    return 0;
err:
    return ~0;
}

int HttpServer::stop ()
{
    LIBENCLOUD_SVC_TRACE;
    
    close();

    return 0;
}

void HttpServer::incomingConnection (int sd)
{
    LIBENCLOUD_SVC_TRACE;

    _socket = new QTcpSocket(this); 
    LIBENCLOUD_SVC_ERR_IF (_socket == NULL);

    connect(_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), 
            this, SLOT(error(QAbstractSocket::SocketError)));
    _socket->setSocketDescriptor(sd);

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

    LIBENCLOUD_SVC_DBG("bytesAvailable: " << QString::number(socket->bytesAvailable()));

    inData = socket->readAll();
    LIBENCLOUD_ERR_IF (inData.isEmpty());

    LIBENCLOUD_DBG("<<<" << QString(inData).replace(LIBENCLOUD_HTTP_NL, " | "));

    // if nothing has been received yet we need a header
    if (_contexts[socket].data.isEmpty())
    {
        HttpHeaders headers;
        LIBENCLOUD_ERR_IF (headers.decode(inData));

        LIBENCLOUD_DBG("header size: " << headers.getSize() << 
                ", content-length: " << headers.get("content-length"));
        
        _contexts[socket].bytesExpected = headers.getSize() + headers.get("content-Length").toInt();
    }

    _contexts[socket].data.append(inData);

    // check if message is complete
    if (_contexts[socket].data.size() >= _contexts[socket].bytesExpected)
    {
        outData = handleMessage(_contexts[socket].data);
        LIBENCLOUD_ERR_IF (outData.isEmpty());

        LIBENCLOUD_DBG(">>>" << QString(outData).replace(LIBENCLOUD_HTTP_NL, " | "));

        socket->write(outData);
        _contexts[socket].data.clear();
    }

err:
    return;
}

void HttpServer::disconnected ()
{
    LIBENCLOUD_SVC_TRACE;

    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}

void HttpServer::error (QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    QTcpSocket* socket = (QTcpSocket*)sender();

    LIBENCLOUD_SVC_DBG("err: " << QString::number(socket->error())
            << " (" << socket->errorString() << ")");
}

//
// private methods
//

QByteArray HttpServer::handleMessage (QByteArray message)
{
    LIBENCLOUD_SVC_TRACE;

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
