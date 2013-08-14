#ifndef _ECE_UTILS_H
#define _ECE_UTILS_H

#include <QtCore>
#include <QString>
#include <QDateTime>
#include "ece.h"

namespace EceUtils {

QString getHwInfo (void);
QDateTime pytime2DateTime (QString pydate);

} // namespace EceUtils

#endif
