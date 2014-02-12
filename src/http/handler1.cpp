#include <encloud/HttpHandler>
#include <common/common.h>
#include <common/config.h>
#include <http/handler.h>
#include <http/handler1.h>

//
// defines
//

// path to GET info: "state", and "need" (license, auth)
#define LIBENCLOUD_HANDLER_STATUS_PATH         "status"

// setup operations: SET license (SECE), GET PoI (ECE)
#define LIBENCLOUD_HANDLER_SETUP_PATH          "setup"

// cloud operations: POST actions (open, close, syncRoutes)
#define LIBENCLOUD_HANDLER_CLOUD_PATH          "cloud"

namespace libencloud {

//
// public methods
//

ApiHandler1::ApiHandler1 (HttpHandler *parent)
    : _parent(parent)
{

}

int ApiHandler1::handle (const HttpRequest &request, HttpResponse &response)
{
    QString path;
    QString version;
    QRegExp versionRx(LIBENCLOUD_HANDLER_PATH_REGEX);
    QString action;

    LIBENCLOUD_TRACE;

    path = QDir(request.getUrl()).absolutePath();
    LIBENCLOUD_ERR_IF (path.indexOf(versionRx) < 0);

    version = versionRx.cap(1);
    LIBENCLOUD_ERR_IF (version.toInt() != API_VERSION_1);

    action = versionRx.cap(2);

    response.getHeaders()->set("Content-Type", "application/json");

    if (action == LIBENCLOUD_HANDLER_STATUS_PATH)
        return _handle_status(request, response);
    else if (action == LIBENCLOUD_HANDLER_SETUP_PATH)
        return _handle_setup(request, response);
    else if (action == LIBENCLOUD_HANDLER_CLOUD_PATH)
        return _handle_cloud(request, response);
    else 
        response.setStatus(LIBENCLOUD_HTTP_STATUS_NOTFOUND);

    return 0;
err:
    response.setStatus(LIBENCLOUD_HTTP_STATUS_BADREQUEST);
    return ~0;
}

//
// private methods
//

int ApiHandler1::_handle_status (const HttpRequest &request, HttpResponse &response)
{
    switch (httpMethodFromString(request.getMethod()))
    {
        case LIBENCLOUD_HTTP_METHOD_GET:
            response.setContent(
                    "jsonpCallback({"\
                        "'state' : '" + _parent->getCoreState() +  "', "\
                        "'need' : '" + _parent->getNeed() +  "'"\
                    "})");
            break;
        default:
            response.setStatus(LIBENCLOUD_HTTP_STATUS_BADMETHOD);
            break;
    }

    return 0;
}

int ApiHandler1::_handle_setup (const HttpRequest &request, HttpResponse &response)
{
    return 0;
}

int ApiHandler1::_handle_cloud (const HttpRequest &request, HttpResponse &response)
{
    return 0;
}

}  // namespace libencloud
