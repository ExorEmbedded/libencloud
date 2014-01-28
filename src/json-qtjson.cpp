#include <encloud/common.h>
#include "json.h"
#include "qtjson.h"

/**
 * QtJson-based interface (GPL/self-contained)
 */

namespace encloud {
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
} // namespace encloud
