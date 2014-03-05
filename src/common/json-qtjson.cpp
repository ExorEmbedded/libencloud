#include <encloud/Json>
#include <common/common.h>
#include <common/config.h>
#include "qtjson.h"

/**
 * QtJson-based interface (GPL/self-contained)
 */

namespace libencloud {
namespace json {

QVariant parse (const QString &str, bool &ok)
{
    return QtJson::parse(str, ok);
}

QString serialize (const QVariant &json, bool &ok)
{
    return QtJson::serialize(json, ok);
}

} // namespace json
} // namespace libencloud
