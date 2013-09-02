#include "ece.h"
#include "json.h"
#include "qtjson.h"

/**
 * QtJson-based interface (GPL/self-contained)
 */

namespace EceJson {

QVariant parse (const QString &str, bool &ok)
{
    return QtJson::parse(str, ok);
}

QString serialize (const QVariant &json, bool &ok)
{
    return QtJson::serialize(json, ok);
}

} // namespace
