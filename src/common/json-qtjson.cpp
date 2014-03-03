#include <encloud/Json>
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
