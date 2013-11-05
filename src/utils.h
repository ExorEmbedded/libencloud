#ifndef _ECE_UTILS_H
#define _ECE_UTILS_H

#include <QtCore>
#include <QString>
#include <QDateTime>
#include "ece.h"

namespace EceUtils {

ECE_DLLSPEC QString getHwInfo (void);
ECE_DLLSPEC QDateTime pytime2DateTime (QString pydate);
ECE_DLLSPEC QByteArray encodeQueryItem (QString pydate);
ECE_DLLSPEC const char *file2Data (QFileInfo fi);

} // namespace EceUtils

#endif
