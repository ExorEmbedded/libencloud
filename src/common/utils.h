#ifndef _LIBENCLOUD_PRIV_UTILS_H
#define _LIBENCLOUD_PRIV_UTILS_H

#include <QDateTime>
#include <QString>
#include <QtCore>
#include <QUuid>
#include <encloud/Common>

namespace libencloud {
namespace utils {

LIBENCLOUD_DLLSPEC QString getHwInfo (void);
LIBENCLOUD_DLLSPEC QDateTime pytime2DateTime (QString pydate);
LIBENCLOUD_DLLSPEC QByteArray encodeQueryItem (QString pydate);
LIBENCLOUD_DLLSPEC const char *file2Data (QFileInfo fi);
LIBENCLOUD_DLLSPEC QString uuid2String (const QUuid &uuid);
LIBENCLOUD_DLLSPEC char *ustrdup (const char *s);
LIBENCLOUD_DLLSPEC int execute (QString path, QStringList args, QString &out);

} // namespace utils
} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_HELPERS_H_ */
