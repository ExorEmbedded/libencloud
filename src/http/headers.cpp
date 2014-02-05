#include <encloud/HttpHeaders>
#include <common/common.h>
#include <common/config.h>

namespace libencloud 
{

//
// public methods
//

HttpHeaders::HttpHeaders ()
{
    LIBENCLOUD_TRACE;
}

HttpHeaders::~HttpHeaders ()
{
    LIBENCLOUD_TRACE;
}

int HttpHeaders::decode (QStringList lines)
{
    LIBENCLOUD_TRACE;

    _map.clear();

    foreach (QString line, lines)
    {
        if (line == "")
            continue;

        QStringList keyval = line.split(':');
        if (keyval.count() != 2)
            continue;

        set(keyval[0].trimmed(), keyval[1].trimmed());
    }

    LIBENCLOUD_DBG("headers: " << _map);

    return 0;
}

QString HttpHeaders::get (const QString &key)
{ 
    return _map[key.toLower()]; 
}

void HttpHeaders::set (const QString &key, const QString &value)
{ 
    _map[key.toLower()] = value; 
}

QMap<QString, QString> *HttpHeaders::getMap()
{ 
    return &_map;
}

}  // namespace libencloud
