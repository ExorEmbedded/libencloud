#include <encloud/Http/Http>
#include <encloud/Http/HttpHeaders>
#include <common/common.h>
#include <common/config.h>

// disable heavy tracing
#undef LIBENCLOUD_TRACE 
#define LIBENCLOUD_TRACE do {} while(0)

namespace libencloud {

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

int HttpHeaders::decode (const QByteArray &data)
{
    LIBENCLOUD_TRACE;

    QStringList lines;
    QStringList fields;

    const char *endHdr = LIBENCLOUD_HTTP_NL LIBENCLOUD_HTTP_NL;

    //LIBENCLOUD_DBG("data: " << data);

    lines = QString(data).split(LIBENCLOUD_HTTP_NL);
    LIBENCLOUD_ERR_IF (lines.count() == 0);

    fields = lines[0].split(' ');
    LIBENCLOUD_ERR_IF (fields.count() < 3);

    _method = fields[0];
    _url = fields[1];
    _version = fields[2];

    lines.removeFirst(); 
    lines.removeLast();

    _size = data.indexOf(endHdr) + strlen(endHdr);

    if (lines.count())
        LIBENCLOUD_ERR_IF (decode(lines));

    return 0;
err:
    return ~0;

    return 0;
}

int HttpHeaders::decode (const QStringList &lines)
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

    //LIBENCLOUD_DBG("headers: " << _map);

    return 0;
}

qint64 HttpHeaders::getSize() const         { return _size; }
QString HttpHeaders::getMethod() const      { return _method; }
QString HttpHeaders::getUrl() const         { return _url; }
QString HttpHeaders::getVersion () const    { return _version; }

QString HttpHeaders::get (const QString &key) const
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
