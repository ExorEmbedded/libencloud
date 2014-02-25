#include <QByteArray>
#include <encloud/HttpResponse>
#include <common/common.h>
#include <common/config.h>

// disable heavy tracing
#undef LIBENCLOUD_TRACE 
#define LIBENCLOUD_TRACE do {} while(0)

namespace libencloud 
{

//
// public methods
//

HttpResponse::HttpResponse ()
{
    LIBENCLOUD_TRACE;

    // set defaults
    _version = "HTTP/1.1";

    // consider status not being set an internal error
    _status = LIBENCLOUD_HTTP_STATUS_INTERNALERROR;

    // default for errors is no content
    //_headers.set("Content-Type", "text/html");
}

HttpResponse::~HttpResponse ()
{
    LIBENCLOUD_TRACE;
}

HttpHeaders *HttpResponse::getHeaders () { return &_headers; }

void HttpResponse::setContent (const QString &content)
{
    _content = content;
}

void HttpResponse::setStatus (HttpStatus status)
{
    _status = status;  
}

int HttpResponse::encode (QByteArray &data)
{
    LIBENCLOUD_TRACE;

    data.append(_version + " " + QString::number(_status) + " " +
            httpStatusToString(_status) + "\r\n");

    _headers.set("Content-Length", QString::number(_content.size()));

    QMapIterator<QString, QString> i(*_headers.getMap());

    while (i.hasNext()) {
        i.next();
        data.append(i.key() + ": " + i.value() + "\r\n");
    }
    data.append("\r\n");
    data.append(_content);
    data.append("\r\n");

    return 0;
}

}  // namespace libencloud
