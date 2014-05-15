#ifndef _LIBENCLOUD_PRIV_HTTP_HANDLER1_H_
#define _LIBENCLOUD_PRIV_HTTP_HANDLER1_H_

#include <encloud/Http/HttpAbstractHandler>

/**
 * Refer to doc/APIv1.
 */

namespace libencloud {

class HttpHandler;

// Not a QObject because Handler object is used externally
class ApiHandler1 : public HttpAbstractHandler 
{
public:
    ApiHandler1 (HttpHandler *parent);
    int handle (const HttpRequest &request, HttpResponse &response);

private:
    int _handle_status (const HttpRequest &request, HttpResponse &response);
    int _handle_auth (const HttpRequest &request, HttpResponse &response);
    int _handle_setup (const HttpRequest &request, HttpResponse &response);
    int _handle_cloud (const HttpRequest &request, HttpResponse &response);
    void _setContent (const HttpRequest &request, HttpResponse &response, const QString &content);

    HttpHandler *_parent;
};

}  // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_HTTP_HANDLER1_H_ */
