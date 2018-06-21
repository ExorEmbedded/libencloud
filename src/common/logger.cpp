#include <encloud/Logger>
#include <encloud/Utils>
#include <common/common.h>
#include <common/config.h>
#include <QTcpSocket>

// logging: Qt4 forces usage of globals with qInstallMsgHandler()
static QTextStream *g_svcLogText = NULL;
static void __log_handler (QtMsgType type, const char *msg);
static QtMsgHandler __default_log_handler = NULL;

namespace libencloud {

//
// [Logger]
//

//
// public methods
//

Logger::Logger ()
    : _isValid(false)
{
    LIBENCLOUD_TRACE;

    // set defaults
    _path = LIBENCLOUD_LOGGER_PATH;
    _extraMode = QFile::NotOpen;
}

Logger::~Logger ()
{
    LIBENCLOUD_TRACE;

    close();
}

int Logger::setPath (const QString &path)
{
    _path = path;

    return 0;
}

int Logger::setExtraMode (QIODevice::OpenModeFlag flags)
{
    _extraMode = flags;

    return 0;
}

int Logger::open ()
{
    QtMsgHandler prev_log_handler;
    LIBENCLOUD_DBG("[Logger] path: " << _path);

    if (QFile::exists(_path))
    {
        QFileInfo pathInfo(_path);
        QString prevPath = pathInfo.absolutePath() + "/" + pathInfo.baseName() + ".0." + pathInfo.completeSuffix();
        QFile::remove(prevPath);
        QFile::rename(_path, prevPath);
    }

    _logFile.setFileName(_path);
    LIBENCLOUD_ERR_IF (!utils::fileCreate(_logFile, QIODevice::WriteOnly | QIODevice::Text | _extraMode));

    _logText.setDevice(&_logFile);
    g_svcLogText = &_logText;

    prev_log_handler = qInstallMsgHandler(__log_handler);
    // Save default qt log handler
    if (prev_log_handler != __log_handler) {
        __default_log_handler = prev_log_handler;
    }

    _isValid = true;

    LIBENCLOUD_TRACE;

    return 0;
err:
    if (_logFile.error())
        LIBENCLOUD_DBG("[Logger] error: " << QString::number(_logFile.error()) <<
                    " (" << _logFile.errorString() << ")");
    _isValid = false;
    return ~0;
}

int Logger::close ()
{
    LIBENCLOUD_TRACE;

    if (_isValid)
    {
        _logFile.flush();
        g_svcLogText = NULL;
        _logFile.close();
    }

    _isValid = false;

    return 0;
}

bool Logger::isValid ()
{
    return _isValid;
}

bool Logger::connected ()
{
    return ((_client != NULL) && 
            (_client->state() == QAbstractSocket::ConnectedState));
}

int Logger::connectToListener (const QString &surl)
{
    LIBENCLOUD_DBG("[Logger] Connecting to listener: " << surl);

    QUrl url(surl);

    LIBENCLOUD_DELETE(_client);

    _client = new QTcpSocket();
    LIBENCLOUD_ERR_IF (_client == NULL);

    _client->connectToHost(QHostAddress(url.host()), url.port(), QIODevice::WriteOnly);

    return 0;
err:
    return ~0;
}

QTcpSocket *Logger::_client = NULL;

int Logger::send (const QString &log)
{
    if (_client == NULL ||
            _client->state() != QAbstractSocket::ConnectedState)
        return ~0;

    _client->write(qPrintable(log));

    return 0;
}

//
// protected slots
//

void Logger::_clientStateChanged (QAbstractSocket::SocketState socketState)
{
    LIBENCLOUD_UNUSED(socketState);

    //LIBENCLOUD_DBG("[Logger] state: " << QString::number(socketState));
}

void Logger::_clientConnected ()
{
    LIBENCLOUD_TRACE;
}

void Logger::_clientError (QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    QTcpSocket *socket = (QTcpSocket*) sender();

    switch (socket->error())
    {
        // silent errors
        case QAbstractSocket::RemoteHostClosedError:
            break;
        default:
            LIBENCLOUD_DBG("[Logger] error: " << QString::number(socket->error())
                    << " (" << socket->errorString() << ")");
    }

    socket->deleteLater();
}

//
// [LogListener]
//

LogListener::LogListener ()
{
    LIBENCLOUD_TRACE;
}

LogListener::~LogListener ()
{
    LIBENCLOUD_TRACE;
}

int LogListener::start (const QHostAddress &address, quint16 port)
{
    LIBENCLOUD_TRACE;

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

    LIBENCLOUD_DBG("[LogListener] Bound to port: " << QString::number(port));

    emit portBound(port);

    return 0;
err:
    return ~0;
}

void LogListener::incomingConnection (int sd)
{
    LIBENCLOUD_TRACE;

    QTcpSocket *socket = new QTcpSocket(this);
    LIBENCLOUD_ERR_IF (socket == NULL);

    connect(socket, SIGNAL(readyRead()), this, SLOT(_socketReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(_socketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), 
            this, SLOT(_socketError(QAbstractSocket::SocketError)));
    socket->setSocketDescriptor(sd);

err:
    return;
}

int LogListener::stop ()
{
    LIBENCLOUD_TRACE;

    if (isListening())
        close();

    return 0;
}

void LogListener::_socketReadyRead ()
{
    enum { MAX_LINE = 1024 };
    char line[MAX_LINE];
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

    while (socket->readLine(line, sizeof(line)) > 0)
        emit log(line);
}

void LogListener::_socketDisconnected ()
{
    LIBENCLOUD_TRACE;

    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    socket->deleteLater();
}

void LogListener::_socketError (QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

    LIBENCLOUD_DBG("[LogListener] error: " << QString::number(socket->error())
            << " (" << socket->errorString() << ")");

    socket->deleteLater();
}

}  // namespace libencloud

//
// static functions
//

static void __log_handler (QtMsgType type, const char *msg)
{
    Q_UNUSED(type);

    if (msg == NULL ||
            g_svcLogText == NULL)
        return;

#ifdef Q_OS_WINCE // log rotation
    static int totBytes = 0;
    totBytes += strlen(msg);
    if (totBytes > 200*1024) { // 200KB size rotation -> save old file in .0 and reopen a new file
        QFile* f = (QFile*)g_svcLogText->device();
        QString filename(f->fileName());
        bool rc = QFile::remove(filename + ".0");
        rc = QFile::copy(filename, filename + ".0");
        f->close();
        f->open(QIODevice::WriteOnly | QIODevice::Truncate);
        g_svcLogText->setDevice(f);
        totBytes = 0;
    }
#endif

    *g_svcLogText << msg << endl;

    if (__default_log_handler) {
        __default_log_handler(type, msg);
    }
}
