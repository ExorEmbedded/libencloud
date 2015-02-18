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
#if defined LIBENCLOUD_MODE_QCC || defined LIBENCLOUD_MODE_SECE  // client / self-contained mode
    return (qApp ? qApp->applicationDirPath() : QDir::currentPath()) + sep;
#else  // independent package
    return QString(LIBENCLOUD_BIN_PREFIX);
#endif
}

LIBENCLOUD_DLLSPEC QString getCommonAppDataDir (QString package)
{
    QString s;

#if defined LIBENCLOUD_MODE_QCC || defined LIBENCLOUD_MODE_SECE  // client / self-contained mode

#if defined Q_OS_WIN32
    char szPath[MAX_PATH];
    // compiled for unicode => use ANSI version
    LIBENCLOUD_RETURN_IF (SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath),
            QString());
    s += QString(szPath);
#elif defined Q_OS_MAC || defined Q_OS_UNIX
    s += LIBENCLOUD_DATA_PREFIX;
#endif

    s += sep + QString(LIBENCLOUD_PRODUCTDIR) + sep;
    if (package == "")
        s += QString(LIBENCLOUD_PKGNAME);
    else
        s += package;
    s += sep;

    return s;

#else  // independent package
    LIBENCLOUD_UNUSED(package);
    LIBENCLOUD_UNUSED(s);

    return QString(LIBENCLOUD_DATA_PREFIX);
#endif
}

LIBENCLOUD_DLLSPEC QString getCommonLogDir (QString package)
{
#if defined LIBENCLOUD_MODE_QCC || defined LIBENCLOUD_MODE_SECE  // client / self-contained mode
    return getCommonAppDataDir(package);
#else  // independent package
    LIBENCLOUD_UNUSED(package);
    return QString(LIBENCLOUD_LOG_PREFIX);
#endif
}

}  // namespace libencloud
