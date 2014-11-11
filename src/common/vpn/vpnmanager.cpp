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
    : cfg(config)
    , err(NoError)
    , st(StateDetached)
    , socket(NULL)
    , attachRetries(0)
{
    LIBENCLOUD_TRACE;

    connect(&this->stateTimer, SIGNAL(timeout()), this, SLOT(stateTimeout()));
    connect(&this->connTimer, SIGNAL(timeout()), this, SLOT(connTimeout()));
    this->connTimer.setSingleShot(true);
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
        case AuthFailedError:
            return tr("Authentication failed");
        case ConnTimeout:
            return tr("Connection timeout");
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

    LIBENCLOUD_NOTICE("[VPNManager] Attaching");

    LIBENCLOUD_DBG("[VPNManager] state: " << stateString(this->st) << " host: " << host << " port: " << port);

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

    this->stateTimer.start(LIBENCLOUD_VPNMANAGER_STATE_PERIOD);
    this->connTimer.start(cfg->config.timeout * 1000);

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

    this->stateTimer.stop();
    this->connTimer.stop();

    if (this->socket) 
    {
        //LIBENCLOUD_DBG("[VPNManager] state: " << QString::number(this->socket->state()))

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
        LIBENCLOUD_DBG("[VPNManager] line: " << line);

        QByteArray msgType;
        QByteArray rest;

        line.remove(0,1);

        msgType = line.split(':')[0];
        rest = line.remove(0, msgType.size() + 1);

        if (qstrcmp(msgType, "INFO") == 0) {
            ;
        } else if (qstrcmp(msgType, "PASSWORD") == 0) {
            parseLinePass(rest);
        } else if (qstrcmp(msgType, "LOG") == 0) {
            __LIBENCLOUD_SIMPLE_MSG(-1, "LOG", line.prepend("[VPN] "));
        } else if (qstrcmp(msgType, "FATAL") == 0) {
            LIBENCLOUD_EMIT_ERR(sigError((this->err = MgmtError), rest));
        }
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
//    LIBENCLOUD_DBG("[VPNManager] rest: " << rest);

    if (qstrcmp(rest, "Verification Failed: 'Auth'") == 0)
    {
        if (cfg->config.sslOp.auth == "user-pass")  // plain signal for user
            emit authRequired(Auth::SwitchboardId);
        else if (cfg->config.sslOp.auth == "x509")  // hard error signal
            LIBENCLOUD_EMIT_ERR(sigError((this->err = AuthFailedError), rest));
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
    else if (qstrcmp(rest, "Need 'Private Key' password") == 0)  // PKCS12
    {
        LIBENCLOUD_EMIT_ERR_IF (!_sbAuth.isValid(), authRequired(Auth::SwitchboardId));

        sendAuth("Private Key", "", _sbAuth.getPass());
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
        LIBENCLOUD_DBG("[VPNManager] rest: " << rest);
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
        this->connTimer.stop();
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
    LIBENCLOUD_DBG("[VPNManager] Sending auth type: " << type << " user: " << user << " pass: <not shown>");

    if (this->socket == NULL)
        return;

    QTextStream ts(this->socket);

    if (user != "")
        ts << "username '" << type << "' " << user << "\r\n";

    if (pass != "")
        ts << "password '" << type << "' " << pass << "\r\n";

    this->socket->flush();
}

//
// public slots
//

void VpnManager::authSupplied (const Auth &auth)
{
    LIBENCLOUD_DBG("[VPNManager] type: " << auth.getStrType());

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

void VpnManager::connTimeout ()
{
    LIBENCLOUD_EMIT_ERR(sigError((this->err = ConnTimeout)));
err:
    return;
}

void VpnManager::hostFound ()
{
    LIBENCLOUD_TRACE;
}

void VpnManager::connected ()
{
    LIBENCLOUD_TRACE;

    this->st = StateAttached;

#ifdef Q_OS_WINCE
    enableLogging(true);
#endif
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
    LIBENCLOUD_DBG("[VPNManager] state: " << stateString(this->st) << ", err: " << err);

    // ignore errors if we're already detached
    LIBENCLOUD_RETURN_IF (this->st == StateDetached, );

    switch (err)
    {
        case (QAbstractSocket::ConnectionRefusedError):
            if (this->attachRetries++ == LIBENCLOUD_VPNMANAGER_ATTACH_RETRIES)
            {
                LIBENCLOUD_DBG("[VPNManager] Reached maximum number retries - giving up");
                this->attachRetries = 0;
                LIBENCLOUD_EMIT_ERR(sigError((this->err = SocketError)));
                break;
            }
            LIBENCLOUD_DBG("[VPNManager] Failed connecting to socket - retrying in 1 second");
            QTimer::singleShot(1000, this, SLOT(retryAttach()));
            return; // remain in attaching state
        default:
            // if it exists, previous error is more significant because all
            // errors trigger manager socket error when client is stopped
            if (this->err != NoError)
                LIBENCLOUD_EMIT_ERR(sigError(this->err));
            else
                // e.g. RemoteHostClosedError happens when client fails
                LIBENCLOUD_EMIT_ERR(sigError((this->err = SocketError)));
            break;
    }
err:
    return;
}

void VpnManager::enableLogging(bool enabled)
{
        if (this->socket)
        {
            //LIBENCLOUD_DBG("[VPNManager] state: " << QString::number(this->socket->state()))
            if (this->socket->state() == QAbstractSocket::ConnectedState)
            {
                if (enabled) {
                    this->socket->write("log on all\r\n");
                } else {
                    this->socket->write("log off\r\n");
                }
                this->socket->flush();
            }
        }
}

}


// namespace libencloud
