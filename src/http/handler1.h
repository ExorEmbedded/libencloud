#ifndef _LIBENCLOUD_PRIV_HTTP_HANDLER1_H_
#define _LIBENCLOUD_PRIV_HTTP_HANDLER1_H_

#include <encloud/HttpAbstractHandler>

namespace libencloud {

class ApiHandler1 : public HttpAbstractHandler 
{
public:
    ApiHandler1 (HttpHandler *parent);
    int handle (const HttpRequest &request, HttpResponse &response);

private:
    int _handle_status (const HttpRequest &request, HttpResponse &response);
    int _handle_setup (const HttpRequest &request, HttpResponse &response);
    int _handle_cloud (const HttpRequest &request, HttpResponse &response);

    HttpHandler *_parent;
};

}  // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_HTTP_HANDLER1_H_ */
