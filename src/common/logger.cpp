#include <QDir>
#include <QFileInfo>
#include <encloud/Logger>
#include <common/common.h>
#include <common/config.h>

// logging: Qt4 forces usage of globals with qInstallMsgHandler()
static QTextStream *g_svcLogText = NULL;
static void __log_handler (QtMsgType type, const char *msg);

namespace libencloud {

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
    QFileInfo fi(_path);
    QDir dir(fi.dir());

    LIBENCLOUD_DBG("path: " << _path);

    dir.mkpath(dir.path());

    logFile.setFileName(_path);
    LIBENCLOUD_ERR_IF (!logFile.open(QIODevice::WriteOnly | QIODevice::Text | _extraMode));

    logText.setDevice(&logFile);
    g_svcLogText = &logText;

    qInstallMsgHandler(__log_handler);

    _isValid = true;
    return 0;
err:
    if (logFile.error())
        LIBENCLOUD_DBG("err: " << QString::number(logFile.error()));
    _isValid = false;
    return ~0;
}

int Logger::close ()
{
    LIBENCLOUD_TRACE;

    if (_isValid)
    {
        logFile.flush();
        g_svcLogText = NULL;
        logFile.close();
    }

    _isValid = false;

    return 0;
}

bool Logger::isValid ()
{
    return _isValid;
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

    *g_svcLogText << msg << endl;
}
