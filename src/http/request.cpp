#include <QByteArray>
#include <encloud/HttpRequest>
#include <common/common.h>
#include <common/config.h>

namespace libencloud 
{

//
// public methods
//

HttpRequest::HttpRequest ()
{
    LIBENCLOUD_TRACE;
}

HttpRequest::~HttpRequest ()
{
    LIBENCLOUD_TRACE;
}

int HttpRequest::decode (const QByteArray &data)
{
    LIBENCLOUD_TRACE;

    QStringList lines;
    QStringList fields;

    lines = QString(data).split("\r\n");
    LIBENCLOUD_ERR_IF (lines.count() == 0);

    //LIBENCLOUD_DBG("lines: " << lines);

    fields = lines[0].split(' ');
    LIBENCLOUD_ERR_IF (fields.count() < 3);

    _method = fields[0];
    _url = fields[1];
    _version = fields[2];

    lines.removeFirst(); 
    LIBENCLOUD_ERR_IF (lines.count() == 0);
    LIBENCLOUD_ERR_IF (_headers.decode(lines));

    return 0;
err:
    return ~0;
}

QString HttpRequest::getMethod() const      { return _method; }
QString HttpRequest::getUrl() const         { return _url; }
QString HttpRequest::getVersion () const    { return _version; }

}  // namespace libencloud
