#ifndef _LIBENCLOUD_PRIV_UTILS_H
#define _LIBENCLOUD_PRIV_UTILS_H

#include <QDateTime>
#include <QFile>
#include <QString>
#include <QtCore>
#include <QUuid>
#include <encloud/Common>

namespace libencloud {
namespace utils {

LIBENCLOUD_DLLSPEC QString getHwInfo (void);

LIBENCLOUD_DLLSPEC QDateTime pytime2DateTime (QString pydate);

LIBENCLOUD_DLLSPEC QByteArray encodeQueryItem (QString pydate);

LIBENCLOUD_DLLSPEC bool fileCreate (QFile &file, QFile::OpenMode mode);
LIBENCLOUD_DLLSPEC const char *file2Data (QFileInfo fi);

LIBENCLOUD_DLLSPEC QString bool2String (bool b);
LIBENCLOUD_DLLSPEC int string2Bool (const QString &s, bool &b);
LIBENCLOUD_DLLSPEC QString uuid2String (const QUuid &uuid);
LIBENCLOUD_DLLSPEC char *ustrdup (const char *s);

LIBENCLOUD_DLLSPEC int execute (QString path, QStringList args, QString &out, bool wait = true);

} // namespace utils
} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_HELPERS_H_ */
