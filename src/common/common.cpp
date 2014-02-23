#include <QObject>
#ifdef Q_OS_WIN32
#  include <shlobj.h>
#endif
#include <encloud/Common>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

LIBENCLOUD_DLLSPEC QString getCommonAppDataDir (QString package)
{
#ifdef Q_OS_WIN32
    const QString sep = "/";
    QString s;
    char szPath[MAX_PATH];

    // compiled for unicode => use ANSI version
    LIBENCLOUD_ERR_IF (SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath));

    s += QString(szPath) + sep + QString(LIBENCLOUD_PRODUCTDIR) + sep;

    if (package == "")
        s += QString(LIBENCLOUD_PKGNAME);
    else
        s += package;

    return s;
#else 
    return QString(LIBENCLOUD_DATA_PREFIX);
#endif
err:
    return QString();
}

}  // namespace libencloud
