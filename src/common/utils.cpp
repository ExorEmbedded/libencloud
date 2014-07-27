#include <encloud/Crypto>
#include <encloud/Utils>
#include "helpers.h"
#include "config.h"

namespace libencloud {
namespace utils {

/** \brief Hardware info used for Subject CN which has maximum 64 chars => return an MD5 checksum! */
QString getHwInfo (void)
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
    LIBENCLOUD_ERR_IF (hwInfo.isEmpty());

    LIBENCLOUD_DBG("hwInfo=" << hwInfo);

    LIBENCLOUD_ERR_IF ((s = libencloud_crypto_md5(NULL, (char *) qPrintable(hwInfo), hwInfo.size())) == NULL);
    res = QString(s);

    free(s);
    return res;
err:
    if (s)
        free(s);
    return NULL;
}

QDateTime pytime2DateTime (QString pydate)
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

LIBENCLOUD_DLLSPEC const char *file2Data (QFileInfo fi)
{
    if (!fi.isFile())
        return NULL;

    QByteArray ba;
    QString fn = fi.absoluteFilePath();
    LIBENCLOUD_DBG("fn=" << fn);
    QFile f(fn);

    LIBENCLOUD_ERR_IF (!f.open(QIODevice::ReadOnly));
    LIBENCLOUD_ERR_IF ((ba = f.readAll()).isEmpty());

    return QString(ba.trimmed()).toLocal8Bit();
err:
    return NULL;
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

LIBENCLOUD_DLLSPEC char *ustrdup (const char *s)
{
    return (s == NULL ? NULL :
#ifdef Q_OS_WIN32
                        // avoid ISO C++ warning
                        _strdup(s)
#else
                        strdup(s)
#endif
                        );
}

int execute (QString path, QStringList args, QString &out, bool wait, bool debug)
{
    QProcess p;

    if (debug)
        LIBENCLOUD_DBG(qPrintable(path) << " " << qPrintable(args.join(" ")));

    p.start(path, args);

    if (wait)
    {
        p.waitForFinished(-1);  // failure not critical

        LIBENCLOUD_ERR_IF (p.exitStatus() != QProcess::NormalExit ||
                p.exitCode());
    }

    out = p.readAll();

    if (debug)
        LIBENCLOUD_DBG("out: " << out);

    return 0;
err:
    return ~0;
}

static QVariantMap _mapMerge (const QVariantMap &m1, const QVariantMap &m2)
{
    QVariantMap m = m1;

    for (QVariantMap::const_iterator iter = m2.begin();
        iter != m2.end();
        ++iter)
        m[iter.key()] = iter.value();

    return m;
}

LIBENCLOUD_DLLSPEC void variantMerge (QVariant &to, const QVariant &from)
{
    QVariantMap mTo = to.toMap();
    QVariantMap mFrom = from.toMap();

    for (QVariantMap::const_iterator iter = mFrom.begin();
        iter != mFrom.end();
        ++iter)
    {
        if (iter.value().canConvert<QVariantMap>())
        {
            variantMerge(
                    qvariant_cast<QVariantMap>(to)[iter.key()],
                    iter.value()
                    );

            mTo[iter.key()] = _mapMerge(
                    mTo[iter.key()].toMap(),
                    iter.value().toMap()
                    );
        }
        else
        {
            mTo[iter.key()] = iter.value();
        }
    }

    to = mTo;
}

} // namespace utils
} // namespace libencloud
