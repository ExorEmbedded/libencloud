#ifndef _LIBENCLOUD_PRIV_JSON_H_
#define _LIBENCLOUD_PRIV_JSON_H_

#include <QVariant>
#include <QString>
#include <encloud/Common>
#include "common.h"

namespace libencloud {
namespace json {

    // implementation-specific interfaces (defined in json-xxx.cpp)
    LIBENCLOUD_DLLSPEC QVariant parse (const QString &str, bool &ok);
    LIBENCLOUD_DLLSPEC QString serialize (const QVariant &json, bool &ok);

    // generic helpers (defined in json.cpp)
    LIBENCLOUD_DLLSPEC QVariant parseFromFile (const QString &filename, bool &ok);

} // namespace json
} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_HELPERS_H_ */
