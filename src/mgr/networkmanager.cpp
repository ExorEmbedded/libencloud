#include <QHostAddress>
#include <QRegExp>
#include <QTimer>
#include <encloud/Manager/NetworkManager>
#include <common/common.h>
#include <common/config.h>

namespace libencloud
{

//
// public methods
// 

NetworkManager::NetworkManager (ProcessManager *processManager)
    : _processManager(NULL)
    , _processManagerInternal(false)
    , _process(NULL)
    , _routesOperation(NoneOperation)
{
    LIBENCLOUD_TRACE;

    if (processManager == NULL)
    {
        _processManager = new ProcessManager();
        LIBENCLOUD_ERR_IF (_processManager == NULL);

        _processManagerInternal = true;
    }

    // default to no fallback (primary gateway)
    setFallback(false);
err:
    return;
}

NetworkManager::~NetworkManager ()
{
    LIBENCLOUD_TRACE;

    if (_processManagerInternal)
        LIBENCLOUD_DELETE(_processManager);
}

void NetworkManager::stop ()
{
    LIBENCLOUD_TRACE;

    if (_gateway == "")
        return;

    // remove all routes associated with current gateway (synchronous)
    readRoutes(RemoveRoutesOperation, true);
}

QString NetworkManager::errorString (Error err)
{
    switch (err)
    {
        case NoError:
            return tr("No error");
        case BadParamsError:
            return tr("Bad parameters");
        case RequestError:
            return tr("Error in request");
        case ReplyError:
            return tr("Error in reply");
        case SysError:
            return tr("System error");
    }
    return "";
}

void NetworkManager::setGateway (const QString &addr)
{
    LIBENCLOUD_DBG("[Network] gateway addr: " << addr);

    LIBENCLOUD_ERR_IF (QHostAddress(addr).isNull());

    _gateway = addr;
err:
    return;
}

void NetworkManager::setFallbackGateway (const QString &addr)
{
    LIBENCLOUD_DBG("[Network] fallback addr: " << addr);

    LIBENCLOUD_ERR_IF (QHostAddress(addr).isNull());

    _fallbackGateway = addr;
err:
    return;
}

void NetworkManager::setFallback (bool fallback)
{
    if (!fallback)
        _gw = _gateway;
    else 
        _gw = _fallbackGateway;
}

void NetworkManager::syncRoutes (const QStringList &connectedEndpoints)
{
    LIBENCLOUD_RETURN_IF (_gw == "", );

    LIBENCLOUD_DBG("[Network] Syncing routes gateway: " << _gw << ", endpoints: " << 
            connectedEndpoints.join(","));

    _connectedEndpoints = connectedEndpoints;

    // read routes asynchronously and sync routes
    readRoutes(UpdateRoutesOperation);
}

//
// protected slots
//

void NetworkManager::finishedReadRoutes (int exitCode, QProcess::ExitStatus exitStatus)
{
    //LIBENCLOUD_TRACE;

    LIBENCLOUD_EMIT_RETURN_IF (exitCode || exitStatus || _process == NULL,
            sigError(BadParamsError), );

    // read current routes from routing table
    readRoutesEx(_process);

    // unblock further readRoutes()
    _process = NULL;
}

//
// protected methods
//

void NetworkManager::readRoutesEx (QProcess *process)
{
    //LIBENCLOUD_TRACE;

    LIBENCLOUD_EMIT_RETURN_IF (process == NULL,
            sigError(BadParamsError), );

    // read current routes from routing table
    QString output = process->readAllStandardOutput();
    QRegExp regex("(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\d+)");
    QStringList currentRoutes;

    //LIBENCLOUD_DBG("output: " << output);

    foreach (QString line, output.split('\n'))
    {
        if (regex.indexIn(line) != -1)
        {
            QString ip = regex.cap(1);
            QString mask = regex.cap(2);
            QString gateway = regex.cap(3);
            QString interface = regex.cap(4);
            QString metric = regex.cap(5);

            if (_routesOperation == RemoveRoutesOperation &&
                gateway == _gateway)
                delRoute(ip);
            else
                currentRoutes.append(ip);
        }
    }


    if (_routesOperation != UpdateRoutesOperation)
        return;
    // else (_routesOperation == UpdateRoutesOperation) 

    LIBENCLOUD_DBG("[Network] previous: " << _previousEndpoints.join(",") <<
                   " current: " << currentRoutes.join(",") <<
                   " connected: " << _connectedEndpoints.join(","));

    if (_connectedEndpoints.empty())
    {
        LIBENCLOUD_DBG("[Network] Clearing previous endpoints");
        _previousEndpoints.clear();
    }

    // remove previous endpoints that are no longer in connectedEndpoints
    foreach (QString endpoint, _previousEndpoints)
    {
        QPair<QString,QString> ipmask = parseNetdef(endpoint);

        if (currentRoutes.contains(ipmask.first) &&
                    !_connectedEndpoints.contains(endpoint))
        {
            delRoute(endpoint);
            _previousEndpoints.removeAll(endpoint);
        }
    }

    // add new endpoints which are not already in routing table
    foreach (QString endpoint, _connectedEndpoints)
    {
        if (endpoint == "")
            continue;

        QPair<QString,QString> ipmask = parseNetdef(endpoint);

        if (!currentRoutes.contains(ipmask.first))
        {
            addRoute(endpoint);
            if (!_previousEndpoints.contains(endpoint))
                _previousEndpoints.append(endpoint);
        }
    }

}

void NetworkManager::readRoutes (Operation op, bool sync)
{
    if (_process)
    {
        LIBENCLOUD_ERR("[Network] another process is already running!");
        return;
    }

    LIBENCLOUD_DBG("[Network] reading routes - sync: " << sync);

    _routesOperation = op;

#ifdef Q_OS_WIN
    _process = _processManager->start("route", "PRINT");
#else
    _process = _processManager->start("netstat", "-rn");
#endif

    if (sync)
    {
        _process->waitForFinished(-1);

        // read current routes from routing table
        readRoutesEx(_process);

        // unblock further readRoutes()
        _process = NULL;
    }
    else
    {
        connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                SLOT(finishedReadRoutes(int, QProcess::ExitStatus)));
    }
}

void NetworkManager::addRoute (const QString &endpoint)
{
    QPair<QString,QString> ipmask = parseNetdef(endpoint);

    LIBENCLOUD_ERR_IF ((ipmask == QPair<QString,QString>()));

#ifdef Q_OS_WIN
    _processManager->start("route", "ADD " + ipmask.first +
            " MASK " + ipmask.second + " " +  _gw);
#else
    if (ipmask.second == "255.255.255.255")
        _processManager->start("route", "add " + ipmask.first +
                " gw " + _gw);
    else
        _processManager->start("route", "add -net " + ipmask.first +
                " netmask " + ipmask.second + " gw " + _gw);
#endif

err:
    return;
}

void NetworkManager::delRoute (const QString &endpoint)
{
    QPair<QString,QString> ipmask = parseNetdef(endpoint);

    LIBENCLOUD_ERR_IF ((ipmask == QPair<QString,QString>()));

#ifdef Q_OS_WIN
    if (ipmask.second == "255.255.255.255")
        _processManager->start("route", "DELETE " + ipmask.first);
    else
        _processManager->start("route", "DELETE " + ipmask.first +
                " MASK " + ipmask.second);
#else
    if (ipmask.second == "255.255.255.255")
        _processManager->start("route", "del " + ipmask.first);
    else
        _processManager->start("route", "del -net " + ipmask.first +
                " netmask " + ipmask.second);
#endif

err:
    return;
}

QPair<QString,QString> NetworkManager::parseNetdef (const QString &netdef)
{
    QStringList ipmask = netdef.split("/");
    QString ip, mask = "255.255.255.255";

    LIBENCLOUD_ERR_IF (ipmask.count() < 1);

    ip = ipmask[0];
    LIBENCLOUD_ERR_IF (QHostAddress(ip).isNull());

    if (ipmask.count() == 2)
    {
        mask = ipmask[1];
        LIBENCLOUD_ERR_IF (QHostAddress(mask).isNull());
    }

    return QPair<QString,QString>(ip, mask);
err:
    return QPair<QString,QString>();
}

}  // namespace libencloud
