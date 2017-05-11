#include <QObject>
#include <QCoreApplication>
#ifdef Q_OS_WIN32
#  include <shlobj.h>
#endif
#include <encloud/Common>
#include <encloud/Utils>
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

/* \brief Where to look for application binaries */
LIBENCLOUD_DLLSPEC QString getBinDir ()
{
#ifndef LIBENCLOUD_SPLITDEPS
    return (qApp ? qApp->applicationDirPath() : QDir::currentPath()) + sep;
#else
    return QString(LIBENCLOUD_BIN_PREFIX);
#endif
}

LIBENCLOUD_DLLSPEC QString getCommonAppDataDir (QString package)
{
    QString s;

#ifndef LIBENCLOUD_SPLITDEPS

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

#else
    LIBENCLOUD_UNUSED(package);
    LIBENCLOUD_UNUSED(s);

    return QString(LIBENCLOUD_DATA_PREFIX);
#endif
}

LIBENCLOUD_DLLSPEC QString getCommonLogDir (QString package)
{
#ifndef LIBENCLOUD_SPLITDEPS
    return getCommonAppDataDir(package);
#else
    LIBENCLOUD_UNUSED(package);
    return QString(LIBENCLOUD_LOG_PREFIX);
#endif
}

// Agent mode
LIBENCLOUD_DLLSPEC QString getActivationCode ()
{
    QFile activationCodeFile(getCommonAppDataDir() + "activation-code");
    LIBENCLOUD_ERR_IF (!activationCodeFile.open(QFile::ReadOnly));

    return activationCodeFile.readLine().trimmed();
err:
    return QString();
}

LIBENCLOUD_DLLSPEC int setActivationCode (const QString &code)
{
    QString path = getCommonAppDataDir() + "activation-code";

    if (code == QString(utils::file2Data(QFileInfo(path))))
        return 0;

    LIBENCLOUD_DBG("Writing to: " << getCommonAppDataDir() + "activation-code");

    LIBENCLOUD_ERR_IF (utils::bytes2File(code.toAscii(), path, true));

    return 0;
err:
    return ~0;
}

}  // namespace libencloud
