#include <QByteArray>
#include <common/common.h>
#include <common/config.h>
#include <encloud/HttpHeaders>
#include <encloud/HttpRequest>

// disable heavy tracing
#undef LIBENCLOUD_TRACE 
#define LIBENCLOUD_TRACE do {} while(0)

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

    LIBENCLOUD_ERR_IF (_headers.decode(data));

    lines = QString(data).split(LIBENCLOUD_HTTP_NL);
    LIBENCLOUD_ERR_IF (lines.count() == 0);

    lines.removeFirst(); 
    _content = lines.takeLast();

    return 0;
err:
    return ~0;
}

QString HttpRequest::getMethod() const      { return _headers.getMethod(); }
QString HttpRequest::getUrl() const         { return _headers.getUrl(); }
QString HttpRequest::getVersion () const    { return _headers.getVersion(); }
const HttpHeaders *HttpRequest::getHeaders () const  { return &_headers; }
const QString *HttpRequest::getContent () const  { return &_content; }

}  // namespace libencloud
