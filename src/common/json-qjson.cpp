#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "json.h"

/**
 * QJson-based interface (LGPL/external)
 */

namespace libencloud {
namespace json {

QVariant parse (const QString &str, bool &ok)
{
    QJson::Parser parser;

    return parser.parse(str.toAscii(), &ok);
}

QString serialize (const QVariant &json, bool &ok)
{
    QJson::Serializer serializer;

    return serializer.serialize(json, &ok);
}

} // namespace json
} // namespace libencloud
