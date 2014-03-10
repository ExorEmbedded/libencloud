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
{
    LIBENCLOUD_TRACE;

    if (processManager == NULL)
    {
        _processManager = new ProcessManager();
        LIBENCLOUD_ERR_IF (_processManager == NULL);

        _processManagerInternal = true;
    }
err:
    return;
}

NetworkManager::~NetworkManager ()
{
    LIBENCLOUD_TRACE;

    if (_processManagerInternal)
        LIBENCLOUD_DELETE(_processManager);
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

void NetworkManager::setGateway (const QString &externalAddr, const QString &internalAddr)
{
    Q_UNUSED(externalAddr);

    LIBENCLOUD_DBG("addr: " << internalAddr);

    _gateway = internalAddr;
}

void NetworkManager::syncRoutes (const QStringList &connectedEndpoints)
{
    LIBENCLOUD_RETURN_IF (_gateway == "", );

    LIBENCLOUD_DBG("gateway: " << _gateway << ", endpoints: " << connectedEndpoints);

    _connectedEndpoints = connectedEndpoints;

    // read routes asynchronously and sync routes in finishedReadRoutes()
    readRoutes();
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
    QString output = _process->readAllStandardOutput();
    QRegExp regex("[0-9]+.[0-9]+.[0-9]+.[0-9]+");
    QStringList currentRoutes;

    //LIBENCLOUD_DBG("output: " << output);

    foreach (QString line, output.split('\n'))
    {
        if (regex.indexIn(line) != -1)
            currentRoutes.append(regex.capturedTexts());
    }

    LIBENCLOUD_DBG("currentRoutes: " << currentRoutes);

    // remove previous endpoints that are no longer in connectedEndpoints
    foreach (QString endpoint, _previousEndpoints)
    {
        if (currentRoutes.contains(endpoint) &&
                    !_connectedEndpoints.contains(endpoint))
        {
            delRoute(endpoint);
            _previousEndpoints.removeAll(endpoint);
        }
    }

    // add enpoints which were not already connected in previousEndpoints
    foreach (QString endpoint, _connectedEndpoints)
    {
        if (!_previousEndpoints.contains(endpoint))
        {
            addRoute(endpoint);
            _previousEndpoints.append(endpoint);
        }
    }

    // unblock further readRoutes()
    _process = NULL;
}

//
// private methods
//

void NetworkManager::readRoutes ()
{
    if (_process)
        return;

#ifdef Q_OS_WIN
    _process = _processManager->start("route", "PRINT");
#else
    _process = _processManager->start("route", "-n");
#endif

    connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(finishedReadRoutes(int, QProcess::ExitStatus)));
}

void NetworkManager::addRoute (const QString &endpoint)
{
#ifdef Q_OS_WIN
    _processManager->start("route", "ADD " + endpoint + " MASK 255.255.255.255 " + _gateway);
#else
    _processManager->start("route", "add " + endpoint + " gw " + _gateway);
#endif
}

void NetworkManager::delRoute (const QString &endpoint)
{
#ifdef Q_OS_WIN
    _processManager->start("route", "DELETE " + endpoint);
#else
    _processManager->start("route", "del " + endpoint);
#endif
}

}  // namespace libencloud
