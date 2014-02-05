#include <QDir>
#include <QRegExp>
#include <encloud/HttpHandler>
#include <common/common.h>
#include <common/config.h>

#if 0
#define LIBENCLOUD_HANDLER_VERSION             1
#define LIBENCLOUD_HANDLER_PREFIX              "/api_v"LIBENCLOUD_HANDLER_VERSION
#define LIBENCLOUD_HANDLER_STATUS_PATH         LIBENCLOUD_HANDLER_PREFIX"/manage/status"
#define LIBENCLOUD_HANDLER_GUI_PATH            LIBENCLOUD_HANDLER_PREFIX"/manage/gui"
#define LIBENCLOUD_HANDLER_SWITCHBOARD_PATH    LIBENCLOUD_HANDLER_PREFIX"/manage/access"
#endif

namespace libencloud {

//
// public methods
//

HttpHandler::HttpHandler ()
{
    LIBENCLOUD_TRACE;
}

HttpHandler::~HttpHandler ()
{
    LIBENCLOUD_TRACE;
}

/* TODO forward request to liblibencloud */
int HttpHandler::handle (const HttpRequest &request, HttpResponse &response)
{
    LIBENCLOUD_TRACE;

#if 0
    bool ok;
    QVariant inJson;
    QVariant outJson;
#endif

#if 0
    QString method = request.getMethod();
    QString path = QDir(request.getUrl()).absolutePath();

    LIBENCLOUD_DBG("method: " << method);
    LIBENCLOUD_DBG("path: " << path);

    QRegExp versionRx("/api_v(\\d+)(/.*)");

    if (path.indexOf(versionRx) < 0)
        response = _handle_error(, method);

    QString version = versionRx.cap(1);
    QString path = versionRx.cap(2);

    ApiHandler apiHandler(version);

    response = apiHandler.handle(path, method);
#endif

#if 0
    if (path.startsWith(LIBENCLOUD_HANDLER_STATUS_PATH))
        response = _handle_status(path, method);

    if (path.startsWith(LIBENCLOUD_HANDLER_STATUS_PATH))
        response = _handle_status(path, method);
    else if (path.startsWith(LIBENCLOUD_HANDLER_GUI_PATH))
        response = _handle_gui(path, method);
    else if (path.startsWith(LIBENCLOUD_HANDLER_SWITCHBOARD_PATH))
        response = _handle_switchboard(path, method);
    else
        response = _handle_error(path, method);
#endif

#if 0
    response.setContent("this is the body 123");
    inJson = json::parse(message, ok);
    LIBENCLOUD_ERR_IF (inJson.isNull() || !ok);
    outJson = handleJson(inJson);
    outMessage = json::serialize(outJson, ok).toAscii();
    LIBENCLOUD_ERR_IF (outJson.isNull() || !ok);
#endif

    return 0;
err:
    return ~0;
}

#if 0
int HttpHandler::handle (const HttpRequest &request, HttpResponse &response)
{

}
#endif

//
// private methods
//

}  // namespace libencloud
