#ifndef _LIBENCLOUD_PRIV_HTTP_HANDLER1_H_
#define _LIBENCLOUD_PRIV_HTTP_HANDLER1_H_

#include <encloud/HttpAbstractHandler>

/**
 * ==================
 * | Encloud API v1 |
 * ==================
 *
 * Special Notes:
 *      - responses return json wrapped in a javascript function (JSONP
 *      handling to bypass same-origin policy), so the returned Content-Type is
 *      application/javascript, not application/json.
 * 
 * 
 * [ State Retrieval ]
 * 
 * GET /api_v1/status
 * 
 *      => "jsonpCallback({
 *              'state' : 'Bad state',          # string representation of state
 *              'need' : 'license auth'         # LIST of space-separated "need" strings
 *          })"                                   to be fulfilled via API
 * 
 * 
 * [ Setup Operations ]
 *
 * GET /api_v1/setup                    [ Note: ECE only! ]
 *
 *      => "jsonpCallback({
 *              'poi' : 'aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee'      # proof of identity
 *          })"
 * 
 * POST /api_v1/setup                   [ Note: SECE only! ]
 * 
 *      params:     'license=aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee'  # license supplied
 * 
 * 
 * [ Cloud Operations ]
 *
 * POST /api_v1/cloud
 * 
 *      params:     'action=start' | 'action=stop'      # start or stop Encloud service
 *
 * POST /api_v1/cloud
 * 
 *      params:     'action=syncRoutes&ips[]=192.168.1.100'   # add routes to given IPs
 * 
 * 
 * Upon error, the following two scenarios are possible:
 *    - if the error is at transport level, a standard HTTP status code is
 *      returned (generally with no content)
 *      HTTP Statuses returned in this version:
 *          - LIBENCLOUD_HTTP_STATUS_OK
 *          - LIBENCLOUD_HTTP_STATUS_BADMETHOD
 *          - LIBENCLOUD_HTTP_STATUS_BADREQUEST
 *          - LIBENCLOUD_HTTP_STATUS_NOTFOUND
 *    - if the failure is at application level, an 'error' field will be
 *      returned in the json message with appropriate description.
 * 
 *      => "jsonpCallback({
 *              'error' : 'Some nasty error'
 *          })"
 */

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
