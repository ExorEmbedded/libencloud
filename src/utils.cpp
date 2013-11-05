#include "utils.h"
#include "helpers.h"
#include "crypto.h"
#include "config.h"

namespace EceUtils {

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
    ECE_ERR_IF (hwInfo.isEmpty());

    ECE_DBG("hwInfo=" << hwInfo);

    ECE_ERR_IF ((s = ece_crypto_md5(NULL, (char *) qPrintable(hwInfo), hwInfo.size())) == NULL);
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
ECE_DLLSPEC QByteArray encodeQueryItem (QString s)
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

ECE_DLLSPEC const char *file2Data (QFileInfo fi)
{
    ECE_RETURN_IF (!fi.isFile(), NULL);

    QByteArray ba;
    QString fn = fi.absoluteFilePath();
    ECE_DBG("fn=" << fn);
    QFile f(fn);

    ECE_ERR_IF (!f.open(QIODevice::ReadOnly));
    ECE_ERR_IF ((ba = f.readAll()).isEmpty());

    return QString(ba.trimmed()).toLocal8Bit();
err:
    return NULL;
}

} //namespace EceUtils
