#include <QObject>
#include <QCoreApplication>
#ifdef Q_OS_WIN32
#  include <shlobj.h>
#endif
#include <encloud/Common>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

static const QString sep = "/";

LIBENCLOUD_DLLSPEC QString paramsFind (const Params &params, const QString &key)
{
    foreach (Param param, params)
        if (param.first == key)
            return param.second;

    return "";
}

/* \brief Where to look for application binaries (based on product) */
LIBENCLOUD_DLLSPEC QString getBinDir ()
{
#ifdef Q_OS_WIN
#ifdef LIBENCLOUD_EXOR
    return (qApp ? qApp->applicationDirPath() : QDir::currentPath()) + sep;
#else
    return qgetenv("ProgramFiles") + sep + \
        QString(LIBENCLOUD_PRODUCTDIR) + sep + \
        QString(LIBENCLOUD_BIN_PREFIX) + sep;
#endif
#else  // Q_OS_UNIX
    return QString(LIBENCLOUD_BIN_PREFIX);
#endif
}

LIBENCLOUD_DLLSPEC QString getCommonAppDataDir (QString package)
{
#ifdef Q_OS_WIN32
    QString s;
    char szPath[MAX_PATH];

    // compiled for unicode => use ANSI version
    LIBENCLOUD_ERR_IF (SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath));

    s += QString(szPath) + sep + QString(LIBENCLOUD_PRODUCTDIR) + sep;

    if (package == "")
        s += QString(LIBENCLOUD_PKGNAME);
    else
        s += package;

    s += sep;

    return s;
err:
    return QString();

#else  // Q_OS_UNIX
    LIBENCLOUD_UNUSED(package);

    return QString(LIBENCLOUD_DATA_PREFIX);
#endif
}

LIBENCLOUD_DLLSPEC QString getCommonLogDir (QString package)
{
#ifdef Q_OS_WIN
    return getCommonAppDataDir(package);
#else  // Q_OS_UNIX
    LIBENCLOUD_UNUSED(package);
    return QString(LIBENCLOUD_LOG_PREFIX);
#endif
}

}  // namespace libencloud
