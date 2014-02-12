#include <encloud/Http>

namespace libencloud {

//
// public methods
//

HttpMethod httpMethodFromString (QString method)
{
    if (method == "OPTIONS")
        return LIBENCLOUD_HTTP_METHOD_OPTIONS;
    else if (method == "GET")
        return LIBENCLOUD_HTTP_METHOD_GET;
    else if (method == "HEAD")
        return LIBENCLOUD_HTTP_METHOD_HEAD;
    else if (method == "POST")
        return LIBENCLOUD_HTTP_METHOD_POST;
    else if (method == "PUT")
        return LIBENCLOUD_HTTP_METHOD_PUT;
    else if (method == "DELETE")
        return LIBENCLOUD_HTTP_METHOD_DELETE;
    else if (method == "TRACE")
        return LIBENCLOUD_HTTP_METHOD_TRACE;
    else if (method == "CONNECT")
        return LIBENCLOUD_HTTP_METHOD_CONNECT;
    else
        return LIBENCLOUD_HTTP_METHOD_NULL;
}


QString httpStatusToString (HttpStatus status)
{
    switch (status)
    {
        case LIBENCLOUD_HTTP_STATUS_NULL:
            return "<UNDEFINED>";

        // 1xx
        case LIBENCLOUD_HTTP_STATUS_CONTINUE:
            return "Continue";
        case LIBENCLOUD_HTTP_STATUS_SWITCHING:
            return "Switching Protocols";

        // 2xx
        case LIBENCLOUD_HTTP_STATUS_OK:
            return "OK";
        case LIBENCLOUD_HTTP_STATUS_CREATED:
            return "Created";
        case LIBENCLOUD_HTTP_STATUS_ACCEPTED:
            return "Accepted";
        case LIBENCLOUD_HTTP_STATUS_NAI:
            return "Non-Authoritative Information";
        case LIBENCLOUD_HTTP_STATUS_NOCONTENT:
            return "No Content";
        case LIBENCLOUD_HTTP_STATUS_RESET:
            return "Reset Content";
        case LIBENCLOUD_HTTP_STATUS_PARTIAL:
            return "Partial Content";

        // 3xx
        case LIBENCLOUD_HTTP_STATUS_MULTIPLE:
            return "Multiple Choices";
        case LIBENCLOUD_HTTP_STATUS_MOVED:
            return "Moved Permanently";
        case LIBENCLOUD_HTTP_STATUS_FOUND:
            return "Found";
        case LIBENCLOUD_HTTP_STATUS_SEEOTHER:
            return "See Other";
        case LIBENCLOUD_HTTP_STATUS_NOTMODIFIED:
            return "Not Modified";
        case LIBENCLOUD_HTTP_STATUS_USEPROXY:
            return "Use Proxy";
        case LIBENCLOUD_HTTP_STATUS_UNUSED:
            return "Unused";
        case LIBENCLOUD_HTTP_STATUS_REDIRECT:
            return "Temporary Redirect";
        case LIBENCLOUD_HTTP_STATUS_BADREQUEST:
            return "Bad Request";
        case LIBENCLOUD_HTTP_STATUS_UNAUTHORIZED:
            return "Unauthorized";
        case LIBENCLOUD_HTTP_STATUS_PAYMENT:
            return "Payment Required";
        case LIBENCLOUD_HTTP_STATUS_FORBIDDEN:
            return "Forbidden";
        case LIBENCLOUD_HTTP_STATUS_NOTFOUND:
            return "Not Found";
        case LIBENCLOUD_HTTP_STATUS_BADMETHOD:
            return "Method Not Allowed";
        case LIBENCLOUD_HTTP_STATUS_NOTACCEPTABLE:
            return "Not Acceptable";
        case LIBENCLOUD_HTTP_STATUS_PROXYAUTH:
            return "Proxy Authentication Required";
        case LIBENCLOUD_HTTP_STATUS_TIMEOUT:
            return "Request Timeout";
        case LIBENCLOUD_HTTP_STATUS_CONFLICT:
            return "Conflict";
        case LIBENCLOUD_HTTP_STATUS_GONE:
            return "Gone";
        case LIBENCLOUD_HTTP_STATUS_LENGTHREQ:
            return "Length Required";
        case LIBENCLOUD_HTTP_STATUS_PRECONDITION:
            return "Precondition Failed";
        case LIBENCLOUD_HTTP_STATUS_REQTOOLARGE:
            return "Request Entity Too Large";
        case LIBENCLOUD_HTTP_STATUS_URITOOLONG:
            return "Request-URI Too Long";
        case LIBENCLOUD_HTTP_STATUS_UNSUPPMEDIA:
            return "Unsupported Media Type";
        case LIBENCLOUD_HTTP_STATUS_REQBADRANGE:
            return "Requested Range Not Satisfiable";
        case LIBENCLOUD_HTTP_STATUS_EXPECTFAILED:
            return "Expectation Failed";

        case LIBENCLOUD_HTTP_STATUS_INTERNALERROR:
            return "Internal Server Error";
        case LIBENCLOUD_HTTP_STATUS_NOTIMPLEMENTED:
            return "Not Implemented";
        case LIBENCLOUD_HTTP_STATUS_GWBAD:
            return "Bad Gateway";
        case LIBENCLOUD_HTTP_STATUS_SVCUNAVAIL:
            return "Service Unavailable";
        case LIBENCLOUD_HTTP_STATUS_GWTIMEOUT:
            return "Gateway Timeout";
        case LIBENCLOUD_HTTP_STATUS_BADVERSION:
            return "HTTP Version Not Supported";
    }

    return "";
}

}  // namespace libencloud
