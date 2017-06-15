#include <QtGlobal>
#ifdef Q_OS_WIN
# include "windows.h"
#endif
#include <encloud/Utils>
#ifndef Q_OS_WINCE
#include <encloud/Crypto>
#endif
#include "helpers.h"
#include "config.h"

namespace libencloud {
namespace utils {

/** \brief Hardware info used for Subject CN which has maximum 64 chars => return an MD5 checksum! */
LIBENCLOUD_DLLSPEC QString getHwInfo (void)
{
    char *s = NULL;
    QString res;

#ifdef Q_OS_UNIX
    QString hwInfo = QString("undefined-on-unix-TODO-test-me-on-W32-SECE");
#endif
#ifdef Q_OS_WIN32
    QString hwInfo = QSettings(QLatin1String("HKLM\\Software\\Microsoft\\Cryptography"), QSettings::NativeFormat).\
                value(QLatin1String("MachineGuid")).toString() + " " + \
            QSettings(QLatin1String("HKLM\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), QSettings::NativeFormat).\
                value(QLatin1String("ProcessorNameString")).toString();
#endif
#ifdef Q_OS_WINCE
    QString hwInfo = QSettings(QLatin1String("HKLM\\Software\\Microsoft\\Cryptography"), QSettings::NativeFormat).\
                value(QLatin1String("MachineGuid")).toString() + " " + \
            QSettings(QLatin1String("HKLM\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), QSettings::NativeFormat).\
                value(QLatin1String("ProcessorNameString")).toString();
#endif
    LIBENCLOUD_ERR_IF (hwInfo.isEmpty());

    LIBENCLOUD_DBG("[Utils] hwInfo: " << hwInfo);

#ifndef Q_OS_WINCE
    LIBENCLOUD_ERR_IF ((s = libencloud_crypto_md5_hex(NULL, (unsigned char *) qPrintable(hwInfo), hwInfo.size())) == NULL);
    res = QString(s);
#else
    res = QString::fromLatin1(QCryptographicHash::hash(hwInfo.toUtf8(), QCryptographicHash::Md5));
#endif

    free(s);
    return res;
err:
    if (s)
        free(s);
    return NULL;
}

LIBENCLOUD_DLLSPEC QDateTime pytime2DateTime (QString pydate)
{
    // grab only finite part of float, convert to int64 then multiply by 1000 for fromMSecsSinceEpoch()
    return QDateTime::fromMSecsSinceEpoch(pydate.split(".")[0].toLongLong(NULL, 10)*1000);
}

/* \brief Differs from addQueryItem() which does not properly handle newlines & CO */
LIBENCLOUD_DLLSPEC QByteArray encodeQueryItem (QString s)
{
    s.replace(QLatin1String("\\"), QLatin1String("\\\\"));
    s.replace(QLatin1String("\""), QLatin1String("\\\""));
    s.replace(QLatin1String("\b"), QLatin1String("\\b"));
    s.replace(QLatin1String("\f"), QLatin1String("\\f"));
    s.replace(QLatin1String("\n"), QLatin1String("\\n"));
    s.replace(QLatin1String("\r"), QLatin1String("\\r"));
    s.replace(QLatin1String("\t"), QLatin1String("\\t"));

    return qPrintable(s);
}

// Open a file, creating parent directory if necessary
LIBENCLOUD_DLLSPEC bool fileCreate (QFile &file, QFile::OpenMode mode)
{
    QFileInfo fi(file);
    QDir dir(fi.dir());

    dir.mkpath(dir.path());

    return file.open(mode);
}

LIBENCLOUD_DLLSPEC int bytes2File (const QByteArray &ba, const QString &path, bool text)
{
    QFile file(path);
    QIODevice::OpenMode openMode = QIODevice::WriteOnly;
    if (text)
        openMode |= QIODevice::Text;

    LIBENCLOUD_ERR_IF (!file.open(openMode));
    LIBENCLOUD_ERR_IF (file.write(ba) == -1);
    file.close();

    QFile::setPermissions(path, QFile::ReadOwner|QFile::WriteOwner);

    return 0;
err:
    if (file.error())
        LIBENCLOUD_ERR("file error: " << file.errorString());
    file.close();
    return ~0;
}

LIBENCLOUD_DLLSPEC const char *file2Data (QFileInfo fi)
{
    if (!fi.isFile())
        return NULL;

    QByteArray ba;
    QString fn = fi.absoluteFilePath();
    LIBENCLOUD_DBG("[Utils] Reading file: " << fn);
    QFile f(fn);

    LIBENCLOUD_ERR_IF (!f.open(QIODevice::ReadOnly));
    LIBENCLOUD_ERR_IF ((ba = f.readAll()).isEmpty());

    return QString(ba.trimmed()).toLocal8Bit();
err:
    return NULL;
}

LIBENCLOUD_DLLSPEC int rmRec (const QString &dir)
{
    QDir d(dir);

    if (!d.exists())
        return ~0;

    Q_FOREACH (QFileInfo info, d.entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        QString path = info.absoluteFilePath();
        LIBENCLOUD_DBG("path: " << path);

        if (info.isDir())
            LIBENCLOUD_ERR_IF (rmRec(path));
        else
            LIBENCLOUD_ERR_IF (!QFile::remove(path));
    }

    d.rmdir(dir);

    return 0;
err:
    return ~0;
}

LIBENCLOUD_DLLSPEC QString bool2String (bool b)
{
    return (b ? "true" : "false");
}

LIBENCLOUD_DLLSPEC int string2Bool (const QString &s, bool &b)
{
    if (QString::compare(s, "true", Qt::CaseInsensitive) == 0)
        b = true;
    else if (QString::compare(s, "false", Qt::CaseInsensitive) == 0)
        b = false;
    else 
        return ~0;  // failure

    return 0;  // success
}

LIBENCLOUD_DLLSPEC QString uuid2String (const QUuid &uuid)
{
    return uuid.toString().remove('{').remove('}').toUpper();
}

// [https://docs.python.org/2/library/base64.html]
// Decode string s using the URL- and filesystem-safe alphabet, which
// substitutes - instead of + and _ instead of / in the standard Base64
// alphabet.
LIBENCLOUD_DLLSPEC QString base642Url (const QString &s)
{
    return QString(s).
              replace("+", "-").
              replace("/", "_").
              replace("=", "");
}

LIBENCLOUD_DLLSPEC char *ustrdup (const char *s)
{
    return (s == NULL ? NULL :
#ifdef Q_OS_WIN
                        // avoid ISO C++ warning
                        _strdup(s)
#else
                        strdup(s)
#endif
                        );
}

LIBENCLOUD_DLLSPEC int executeSync (QString cmd)
{
    LIBENCLOUD_DBG("[Utils] Exec sync: "  << qPrintable(cmd));

    return QProcess::execute(cmd);
}

LIBENCLOUD_DLLSPEC int execute (QString path, QStringList args, QString &out, bool wait, bool debug)
{
    QProcess p;
    QFileInfo fi(path);
    QProcess::ExitStatus exitStatus;
    int exitCode;

    if (debug)
        LIBENCLOUD_DBG("[Utils] Exec "  << qPrintable(path) << " " << qPrintable(args.join(" ")));

    LIBENCLOUD_DBG_IF (!fi.isFile() || !fi.isExecutable());

    p.start(path, args);

    if (wait)
    {
        p.waitForFinished(-1);  // failure not critical

        exitStatus = p.exitStatus();
        exitCode = p.exitCode();

        if (debug)
            LIBENCLOUD_DBG("[Utils] Exec exitStatus: " << QString::number(exitStatus) <<
                           " exitCode: " << QString::number(exitCode));
        
        LIBENCLOUD_ERR_IF (exitStatus != QProcess::NormalExit);
        LIBENCLOUD_ERR_IF (exitCode);
    }

    out = p.readAll();

    if (debug)
        LIBENCLOUD_DBG("[Utils] Exec output: " << out);

    return 0;
err:
    return ~0;
}

void mapMerge (QVariantMap &to, const QVariantMap &from)
{
    //LIBENCLOUD_DBG("to: " << to);
    //LIBENCLOUD_DBG("from: " << from);

    for (QVariantMap::const_iterator iter = from.begin();
        iter != from.end();
        ++iter)
    {
        //LIBENCLOUD_DBG("insert: " << iter.key() << " = " << iter.value());

        QVariantMap fromMap = from[iter.key()].toMap();
        QVariantMap newMap = to[iter.key()].toMap();

        if (!newMap.isEmpty() && !fromMap.isEmpty())
        {

            for (QVariantMap::const_iterator iter2 = fromMap.begin();
                    iter2 != fromMap.end();
                    ++iter2)
                newMap.insert(iter2.key(), iter2.value());

            //LIBENCLOUD_DBG("fromMap: " << fromMap);
            //LIBENCLOUD_DBG("newMap: " << newMap);

            to[iter.key()] = newMap;
        }
        else 
        {
            to[iter.key()] = iter.value();
        }
    }
}

LIBENCLOUD_DLLSPEC bool validIp (const QString &s)
{
    return (QRegExp(QString("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}") +
                "([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$")
        .indexIn(s) == 0);
}

// RFC 1123
LIBENCLOUD_DLLSPEC bool validHost (const QString &s)
{
    return (QRegExp(QString("^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*") +
                "([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$")
        .indexIn(s) == 0);
}

#ifdef Q_OS_WIN 
/*
 * OSVERSIONINFO sample values      major       minor
 *
 *  Windows 2000                    5           0
 *  Windows XP                      5           1
 *  Windows XP x64                  5           2
 *  Windows Vista                   6           0
 *  Windows 7                       6           1
 *  Windows 8.1                     6           3
 *  Windows 10                     10           0
 *
 * Notes: 
 *  - applications not manifested for Windows 8.1 or 10 will return the Windows 8
 *  default value (6.2)
 *  - GetVersionEx() is deprecated - for other needs refer to "Version Helper
 *  APIs"
 */
LIBENCLOUD_DLLSPEC int winVersion (int *major, int *minor)
{
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&osvi))
        return ~0;

    if (major)
        *major = osvi.dwMajorVersion;

    if (minor)
        *minor = osvi.dwMinorVersion;

    //LIBENCLOUD_DBG("major: " << *major << ", minor: " << *minor);

    return 0;
}

LIBENCLOUD_DLLSPEC bool winVersionGe (int major, int minor)
{
    int maj;
    int min;

    if (winVersion(&maj, &min))
        return false;

    if (maj > major)
        return true;

    else if (maj == major && min >= minor)
        return true;

    return false;
}
#endif  // Q_OS_WIN

} // namespace utils
} // namespace libencloud
