#include <QDebug>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <common/vpn/vpnmanager.h>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

VpnManager::VpnManager (const Config *config)
    : config(config)
    , err(NoError)
    , st(StateDetached)
    , socket(NULL)
    , attachRetries(0)
    , stateTimer(NULL)
{
    LIBENCLOUD_TRACE;
}

VpnManager::~VpnManager ()
{
    LIBENCLOUD_TRACE;

    detach();
}

VpnManager::Error VpnManager::error ()
{
    return this->err;
}

QString VpnManager::errorString (Error err)
{
    switch (err)
    {
        case NoError:
            return tr("No error");
        case MemoryError:
            return tr("Out of memory");
        case CommError:
            return tr("Communication error");
        case MgmtError:
            return tr("Management error");
        case UnhandledError:
            return tr("Unhandled functionality");
        case SocketError:
            return tr("Error in socket communication");
    }

    return "";
}

VpnManager::State VpnManager::state ()
{
    return this->st;
}

QString VpnManager::stateString (State st)
{
    switch (st)
    {
        case StateDetached:
            return "detached from client";
        case StateAttaching:
            return "attaching to client";
        case StateAttached:
            return "attached to client";
        case StateDetaching:
            return "detaching from client";
    }
    return "";
}

void VpnManager::attach (VpnClient *client, QString host, int port)
{
    LIBENCLOUD_RETURN_IF (client == NULL, );
    LIBENCLOUD_RETURN_IF (this->st == StateAttached, );

    LIBENCLOUD_DBG("state: " << this->st << " host: " << host << " port: " << port);

    detach(); // cleanup if we're reattaching
    this->st = StateAttaching;

    this->client = client;
    this->host = host;
    this->port = port;

    this->socket = new QTcpSocket(this);
    LIBENCLOUD_ERR_IF (this->socket == NULL);

    connect(this->socket, SIGNAL(hostFound()), this, SLOT(hostFound()));
    connect(this->socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(this->socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(this->socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(this->socket, SIGNAL(readChannelFinished()), this, SLOT(readChannelFinished()));
    connect(this->socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
    connect(this->socket, SIGNAL(error(QAbstractSocket::SocketError)), this, 
            SLOT(socketError(QAbstractSocket::SocketError)));

    this->socket->connectToHost(this->host, this->port);
    LIBENCLOUD_ERR_IF (!this->socket->isValid());

    this->stateTimer = new QTimer(this);
    LIBENCLOUD_ERR_IF (this->stateTimer == NULL);

    connect(this->stateTimer, SIGNAL(timeout()), this, SLOT(stateTimeout()));
    this->stateTimer->start(LIBENCLOUD_VPNMANAGER_STATE_PERIOD);

    return;
err:
    if (this->socket == NULL)
        LIBENCLOUD_EMIT_ERR(sigError((this->err = MemoryError)));
    else 
        LIBENCLOUD_EMIT_ERR(sigError((this->err = SocketError)));
    this->detach();
}

void VpnManager::detach ()
{
    if (this->st == StateDetached)
        return;

    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE (this->stateTimer);

    if (this->socket) 
    {
        LIBENCLOUD_DBG("state: " << QString::number(this->socket->state()))

        if (this->socket->state() == QAbstractSocket::ConnectedState)
        {
            this->socket->write("quit\r\n");
            this->socket->flush();
            this->socket->close();
        }

        this->socket->deleteLater();;
    }

    if (this->st != StateAttaching)
        this->attachRetries = 0;
    this->st = StateDetached;
    this->err = NoError;
}

//
// private methods
//

void VpnManager::parseLine (QByteArray line)
{
//    LIBENCLOUD_DBG("line: " << line);

    line.chop(2);  // remove carriage returns

    if (line.startsWith("END") ||
            line.startsWith("SUCCESS"))
    {
        ; // nop
    }
    else if (line.startsWith('>'))
    {
        LIBENCLOUD_DBG("line: " << line);

        QByteArray msgType;
        QByteArray rest;

        line.remove(0,1);

        msgType = line.split(':')[0];
        rest = line.remove(0, msgType.size() + 1);

        if (qstrcmp(msgType, "INFO") == 0)
            ;
        else if (qstrcmp(msgType, "PASSWORD") == 0)
            parseLinePass(rest);
        else if (qstrcmp(msgType, "FATAL"))
            LIBENCLOUD_EMIT_ERR(sigError((this->err = MgmtError), rest));
    }
    else
    {
        parseLineState(line);
    }
err:
    return;
}

void VpnManager::parseLinePass (QByteArray rest)
{
//    LIBENCLOUD_DBG("rest: " << rest);

    if (qstrcmp(rest, "Verification Failed: 'Auth'") == 0)
    {
        emit authRequired(Auth::SwitchboardId);
    }
    else if (qstrcmp(rest, "Verification Failed: 'HTTP Proxy'") == 0 ||
            qstrcmp(rest, "Verification Failed: 'SOCKS Proxy'") == 0)
    {
        emit authRequired(Auth::ProxyId);
    }
    else if (qstrcmp(rest, "Need 'Auth' username/password") == 0)
    {
        LIBENCLOUD_EMIT_ERR_IF (!_sbAuth.isValid(), authRequired(Auth::SwitchboardId));

        sendAuth("Auth", _sbAuth.getUser(), _sbAuth.getPass());
    }
    else if (qstrcmp(rest, "Need 'HTTP Proxy' username/password") == 0)
    {
        LIBENCLOUD_EMIT_ERR_IF (!_proxyAuth.isValid(), authRequired(Auth::ProxyId));

        sendAuth("HTTP Proxy", _proxyAuth.getUser(), _proxyAuth.getPass());
    }
    else if (qstrcmp(rest, "Need 'SOCKS Proxy' username/password") == 0)
    {
        LIBENCLOUD_EMIT_ERR_IF (!_proxyAuth.isValid(), authRequired(Auth::ProxyId));

        sendAuth("SOCKS Proxy", _proxyAuth.getUser(), _proxyAuth.getPass());
    }
    else
    {
        LIBENCLOUD_EMIT_ERR(sigError((this->err = UnhandledError)));
    }
err:
    return;
}

void VpnManager::parseLineState (QByteArray line)
{
//    LIBENCLOUD_DBG("line: " << line);

    QList<QByteArray> words;
    QByteArray state;
    QByteArray desc;

    words = line.split(',');
    LIBENCLOUD_ERR_MSG_IF (words.size() < 2, line);

    state = words[1];

    if (words.size() >= 3)
        desc = words[2];

    if (qstrcmp(state, "CONNECTING") == 0
            || qstrcmp(state, "RECONNECTING") == 0
            || qstrcmp(state, "TCP_CONNECT") == 0)
    {
        this->client->setState(VpnClient::StateConnecting);
    }
    else if (qstrcmp(state, "WAIT") == 0)
    {
        this->client->setState(VpnClient::StateConnectingWaitingInitialResponse);
    }
    else if (qstrcmp(state, "AUTH") == 0)
    {
        this->client->setState(VpnClient::StateConnectingAuthenticating);
    }
    else if (qstrcmp(state, "GET_CONFIG") == 0)
    {
        this->client->setState(VpnClient::StateConnectingDownloadingConfiguration);
    }
    else if (qstrcmp(state, "ASSIGN_IP") == 0)
    {
        this->client->setState(VpnClient::StateConnectingAssigningIP);

        if (words[3] != this->assignedIp)
        {
            this->assignedIp = words[3];
            emit ipAssigned(this->assignedIp);
        }
    }
    else if (qstrcmp(state, "ADD_ROUTES") == 0)
    {
        this->client->setState(VpnClient::StateConnectingAddingRoutes);
    }
    else if (qstrcmp(state, "CONNECTED") == 0)
    {
        this->client->setState(VpnClient::StateConnected);

        if (words[3] != this->assignedIp)
        {
            this->assignedIp = words[3];
            emit ipAssigned(this->assignedIp);
        }
    }
    else if (qstrcmp(state, "EXITING") == 0)
    {
        this->client->setState(VpnClient::StateExiting);
    }

err:
    return;
}

void VpnManager::sendAuth (const QString type, const QString &user, const QString &pass)
{
    LIBENCLOUD_DBG("type: " << type << " user: " << user << " pass: <not shown>");

    if (this->socket == NULL)
        return;

    QTextStream ts(this->socket);

    ts << "username '" << type << "' " << user << "\r\n";
    ts << "password '" << type << "' " << pass << "\r\n";
    this->socket->flush();
}

//
// public slots
//

void VpnManager::authSupplied (const Auth &auth)
{
    LIBENCLOUD_DBG("type: " << auth.getType());

    switch (auth.getId())
    {
        case Auth::SwitchboardId:
            _sbAuth = auth;
            break;
        case Auth::ProxyId:
            _proxyAuth = auth;
            break;
        default:
            LIBENCLOUD_ERR("bad id");
    }

    return;
}

//
// private slots
//

void VpnManager::retryAttach ()
{
    LIBENCLOUD_RETURN_IF (this->st != StateAttaching, );

    attach(this->client, this->host, this->port);
}

void VpnManager::stateTimeout ()
{
    if (this->socket == NULL || this->st != StateAttached)
        return;

    QTextStream ts(this->socket);

    ts << "state" << "\r\n";

    this->socket->flush();
}

void VpnManager::hostFound ()
{
    LIBENCLOUD_TRACE;
}

void VpnManager::connected ()
{
    LIBENCLOUD_TRACE;

    this->st = StateAttached;
}

void VpnManager::disconnected ()
{
    LIBENCLOUD_TRACE;

    this->st = StateDetaching;
}

void VpnManager::readyRead ()
{
    QByteArray ba;

//    LIBENCLOUD_DBG("bytes: " << this->socket->bytesAvailable());

    while (!(ba = this->socket->readLine()).isEmpty())
        parseLine(ba);
}

void VpnManager::readChannelFinished ()
{
    LIBENCLOUD_TRACE;
}

void VpnManager::bytesWritten (qint64 bytes)
{
    LIBENCLOUD_UNUSED(bytes);

//    LIBENCLOUD_TRACE;
}

void VpnManager::socketError (QAbstractSocket::SocketError err)
{
    LIBENCLOUD_DBG("state: " << QString::number(this->st) << ", err: " << err);

    // ignore errors if we're already detached
    LIBENCLOUD_RETURN_IF (this->st == StateDetached, );

    switch (err)
    {
        case (QAbstractSocket::ConnectionRefusedError):
            if (this->attachRetries++ == LIBENCLOUD_VPNMANAGER_ATTACH_RETRIES)
            {
                LIBENCLOUD_DBG("Reached maximum number retries - giving up");
                this->attachRetries = 0;
                LIBENCLOUD_EMIT_ERR(sigError((this->err = SocketError)));
                break;
            }
            LIBENCLOUD_DBG("Failed connecting to socket - retrying in 1 second");
            QTimer::singleShot(1000, this, SLOT(retryAttach()));
            return; // remain in attaching state
        default:
            // e.g. RemoteHostClosedError happens when client fails
            LIBENCLOUD_EMIT_ERR(sigError((this->err = SocketError)));
            break;
    }
err:
    return;
}

}  // namespace libencloud
