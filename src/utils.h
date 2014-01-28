#ifndef _ENCLOUD_UTILS_H
#define _ENCLOUD_UTILS_H

#include <QtCore>
#include <QString>
#include <QDateTime>
#include "encloud.h"

namespace encloud {
namespace utils {

ENCLOUD_DLLSPEC QString getHwInfo (void);
ENCLOUD_DLLSPEC QDateTime pytime2DateTime (QString pydate);
ENCLOUD_DLLSPEC QByteArray encodeQueryItem (QString pydate);
ENCLOUD_DLLSPEC const char *file2Data (QFileInfo fi);
ENCLOUD_DLLSPEC char *ustrdup (const char *s);

} // namespace utils
} // namespace encloud

#endif
