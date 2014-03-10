#include <encloud/Core>
#include <encloud/Common>
#include <encloud/HttpServer>
#include <encloud/Server>
#include <common/common.h>
#include <common/config.h>

#define LIBENCLOUD_SRV_LISTEN          "127.0.0.1"

namespace libencloud  {

//
// public methods
//

Server::Server (QObject *parent, Mode mode)
    : _parent(parent)
    , _mode(mode)
    , _running(false)
    , _cfg(NULL)  // set by Core during init
    , _core(NULL)
    , _handler(NULL)
    , _localServer(NULL)
    , _cloudServer(NULL)
    , _port(LIBENCLOUD_SRV_PORT_DFT)
{
    LIBENCLOUD_TRACE;

    _core = new libencloud::Core();
    LIBENCLOUD_ERR_IF (_core == NULL);
    LIBENCLOUD_ERR_IF (!_core->isValid());

    _handler = new libencloud::HttpHandler();
    LIBENCLOUD_ERR_IF (_handler == NULL);

    _localServer = new HttpServer(_parent, this); 
    LIBENCLOUD_ERR_IF (_localServer == NULL);

    _localServer->setHandler(_handler);
    connect(_localServer, SIGNAL(portBound(int)), this, SIGNAL(portBound(int)));

    if (_mode == Server::EncloudMode)
    {
        _cloudServer = new HttpServer(_parent, this); 
        LIBENCLOUD_ERR_IF (_cloudServer == NULL);

        _cloudServer->setHandler(_handler);
    }

    return;
err:
    _delete();
}

Server::~Server ()
{
    LIBENCLOUD_TRACE;

    _delete();
}

HttpAbstractHandler *Server::getHandler () { return _handler; }

int Server::setHandler (HttpAbstractHandler *handler)
{
    LIBENCLOUD_ERR_IF (handler == NULL);

    _localServer->setHandler(handler);

    if (_cloudServer)
        _cloudServer->setHandler(handler);

    return 0;
err:
    return ~0;
}

int Server::setInitialPort (int port)
{
    LIBENCLOUD_RETURN_IF (port < 0 || port > 65536, ~0);

    _port = port;

    return 0;
}

//
// public slots
//

void Server::vpnIpAssigned (const QString &ip)
{
    LIBENCLOUD_DBG ("ip: " << ip);

    if (_cloudServer == NULL)
        return;

    if (_cfg && _cfg->config.bind == "all")
        return;

    if (ip != "")
        _cloudServer->start(QHostAddress(ip), _port);
}

//
// protected methods
//

int Server::start ()
{
    LIBENCLOUD_TRACE;

    if (_running)
        return 0;

    LIBENCLOUD_ERR_IF (_core->attachServer(this));

    // Core autostarts only in Encloud Mode
    if (_mode == Server::EncloudMode)
        LIBENCLOUD_ERR_IF (_core->start());

    // initially listens only on local interface, unless configured differently
    if (_cfg && _cfg->config.bind == "all")
    {
        LIBENCLOUD_DBG("Binding to all interfaces");
        _localServer->start(QHostAddress::Any, _port);
    }
    else
    {
        LIBENCLOUD_DBG("Binding only to local interface");
        _localServer->start(QHostAddress(LIBENCLOUD_SRV_LISTEN), _port);
    }

    LIBENCLOUD_ERR_IF (!_localServer->isListening());

    _running = true;

    return 0;
err:
    return ~0;
}

int Server::stop ()
{
    LIBENCLOUD_TRACE;

    if (!_running) 
        return 0;

    _localServer->stop();

    if (_cloudServer)
        _cloudServer->stop();

    _core->stop();

    return 0;
}


//
// private methods
// 

void Server::_delete()
{
    LIBENCLOUD_DELETE(_localServer);
    LIBENCLOUD_DELETE(_cloudServer);
    LIBENCLOUD_DELETE(_handler);
    LIBENCLOUD_DELETE(_core);
}

}  // namespace encloud
