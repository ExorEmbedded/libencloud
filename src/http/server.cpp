#include <QTcpSocket>
#include <QStringList>
#include <QRegExp>
#include <encloud/HttpRequest>
#include <encloud/HttpServer>
#include <common/common.h>
#include <common/config.h>
#include <http/server.h>

#define LIBENCLOUD_SRV_LISTEN          "127.0.0.1"

// disable heavy tracing
#undef LIBENCLOUD_TRACE 
#define LIBENCLOUD_TRACE do {} while(0)

namespace libencloud {

//
// public methods
//
    
HttpServer::HttpServer ()
    : _cfg(NULL)  // set by Core at during init
    , _handler(NULL)
{
    LIBENCLOUD_TRACE;

    _localServer = new HttpServerPriv(this); 
    LIBENCLOUD_ERR_IF (_localServer == NULL);

    _cloudServer = new HttpServerPriv(this); 
    LIBENCLOUD_ERR_IF (_cloudServer == NULL);

err:
    return;
}

HttpServer::~HttpServer ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE(_localServer);
    LIBENCLOUD_DELETE(_cloudServer);
}

HttpAbstractHandler *HttpServer::getHandler () { return _handler; }

int HttpServer::setHandler (HttpAbstractHandler *handler)
{
    LIBENCLOUD_ERR_IF (handler == NULL);

    _handler = handler;

    _localServer->setHandler(handler);
    _cloudServer->setHandler(handler);

    return 0;
err:
    return ~0;
}

int HttpServer::start ()
{
    LIBENCLOUD_TRACE;

    // initially listens only on local interface, unless configured differently
    if (_cfg && _cfg->config.bind == "all")
    {
        LIBENCLOUD_DBG("Binding to all interfaces");
        _localServer->start(QHostAddress::Any,
            LIBENCLOUD_SRV_PORT_DFT);
    }
    else
    {
        LIBENCLOUD_DBG("Binding only to local interface");
        _localServer->start(QHostAddress(LIBENCLOUD_SRV_LISTEN), 
            LIBENCLOUD_SRV_PORT_DFT);
    }

    return 0;
}

int HttpServer::stop ()
{
    LIBENCLOUD_TRACE;

    _localServer->stop();
    _cloudServer->stop();

    return 0;
}

bool HttpServer::isListening ()
{
    return _localServer->isListening();
}

//
// public slots
//

void HttpServer::vpnIpAssigned (const QString &ip)
{
    LIBENCLOUD_DBG ("ip: " << ip);

    if (_cfg && _cfg->config.bind == "all")
        return;

    if (ip != "")
        _cloudServer->start(QHostAddress(ip), LIBENCLOUD_SRV_PORT_DFT);
}

//
// public methods
//

HttpServerPriv::HttpServerPriv (HttpServer *server)
    : QTcpServer(NULL)
    , _server(server)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (server == NULL);
err:
    return;
}

HttpServerPriv::~HttpServerPriv ()
{
    LIBENCLOUD_TRACE;

    close();
}

int HttpServerPriv::setHandler (HttpAbstractHandler *handler)
{
    LIBENCLOUD_ERR_IF (handler == NULL);

    _handler = handler;

    return 0;
err:
    return ~0;
}

int HttpServerPriv::start (const QHostAddress &address, quint16 port)
{
    enum { MAX_ATTEMPTS = 10 };
    int i = 0;

    Config *cfg = _server->_cfg;

    // if port setting already exists use it
    if (cfg && cfg->settings->contains("port"))
    {
        if (listen(address, cfg->settings->value("port").toInt()))
            return 0;

        // otherwise follow through and try other ports
        i++;
    }

    // no setting - scan for a free port
    for (; i < MAX_ATTEMPTS && !listen(address, port); i++, port++) 
        ;
    LIBENCLOUD_ERR_MSG_IF (i == MAX_ATTEMPTS,
            "Could not bind to any port - scanned " <<
            QString::number(MAX_ATTEMPTS) << " ports from " << QString::number(port));

    LIBENCLOUD_DBG("bound to port: " << QString::number(port));

    if (cfg)
    {
        cfg->settings->setValue("port", QString::number(port));
        cfg->settings->sync();
        LIBENCLOUD_ERR_MSG_IF (cfg->settings->status() != QSettings::NoError, 
                "could not write configuration to file - check permissions!");
    }

    return 0;
err:
    return ~0;
}

int HttpServerPriv::stop ()
{
    LIBENCLOUD_TRACE;
    
    if (isListening())
        close();

    return 0;
}

void HttpServerPriv::incomingConnection (int sd)
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

void HttpServerPriv::readyRead ()
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

void HttpServerPriv::disconnected ()
{
    LIBENCLOUD_TRACE;

    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}

void HttpServerPriv::error (QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    QTcpSocket* socket = (QTcpSocket*)sender();

    LIBENCLOUD_DBG("err: " << QString::number(socket->error())
            << " (" << socket->errorString() << ")");
}

//
// private methods
//

QByteArray HttpServerPriv::handleMessage (QByteArray message)
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
