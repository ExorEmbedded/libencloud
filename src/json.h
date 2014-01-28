#ifndef _ENCLOUD_PRIV_JSON_H_
#define _ENCLOUD_PRIV_JSON_H_

#include <QVariant>
#include <QString>
#include <encloud/common.h>
#include "common.h"

namespace encloud {
namespace json {

    // implementation-specific interfaces (defined in json-xxx.cpp)
    ENCLOUD_DLLSPEC QVariant parse (const QString &str, bool &ok);
    ENCLOUD_DLLSPEC QString serialize (const QVariant &json, bool &ok);

    // generic helpers (defined in json.cpp)
    ENCLOUD_DLLSPEC QVariant parseFromFile (const QString &filename, bool &ok);

} // namespace json
} // namespace encloud

#endif  /* _ENCLOUD_PRIV_HELPERS_H_ */
