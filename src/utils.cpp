#include "utils.h"

namespace EceUtils {

/* TESTME @ W32: used for Subject CN - maximum 64 chars! */
QString getHwInfo (void)
{
#ifdef Q_OS_UNIX
    return QString("undefined-on-unix-TODO-test-me-on-W32-SECE");
#endif
#ifdef Q_OS_WIN32
    return QSettings(QLatin1String("HKLM\\Software\\Microsoft\\Cryptography"), QSettings::NativeFormat).\
                value(QLatin1String("MachineGuid")).toString() + " " + \
            QSettings(QLatin1String("HKLM\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), QSettings::NativeFormat).\
                value(QLatin1String("ProcessorNameString")).toString();
#endif
}

QDateTime pytime2DateTime (QString pydate)
{
    // grab only finite part of float, convert to int64 then multiply by 1000 for fromMSecsSinceEpoch()
    return QDateTime::fromMSecsSinceEpoch(pydate.split(".")[0].toLongLong(NULL, 10)*1000);
}

} //namespace EceUtils
