#include <QDir>
#include <QRegExp>
#include <encloud/HttpHandler>
#include <common/common.h>
#include <common/config.h>
#include <http/handler.h>

// disable heavy tracing
#undef LIBENCLOUD_TRACE 
#define LIBENCLOUD_TRACE do {} while(0)

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

QString HttpHandler::getCoreState () const
{
    return _coreState;
}

QString HttpHandler::getNeed () const
{
    return _need;
}

// JSONP support (bypass same-origin policy)
int HttpHandler::handle (const HttpRequest &request, HttpResponse &response)
{
    HttpAbstractHandler *apiHandler = NULL;
    QString path;
    QString version;
    QRegExp versionRx(LIBENCLOUD_HANDLER_PATH_REGEX);
    QString action;

    LIBENCLOUD_TRACE;

    LIBENCLOUD_UNUSED(request);
    LIBENCLOUD_UNUSED(response);

    path = QDir(request.getUrl()).absolutePath();

    LIBENCLOUD_DBG("path: " << path);

    LIBENCLOUD_ERR_IF (path.indexOf(versionRx) < 0);

    version = versionRx.cap(1);
    action = versionRx.cap(2);

    LIBENCLOUD_DBG("version: " << version);
    LIBENCLOUD_DBG("action: " << action);

    apiHandler = _versionToApiHandler((ApiVersion) version.toInt());
    LIBENCLOUD_ERR_IF (apiHandler == NULL);
    LIBENCLOUD_ERR_IF (apiHandler->handle(request, response));

    LIBENCLOUD_DELETE(apiHandler);

    return 0;
err:
    response.setStatus(LIBENCLOUD_HTTP_STATUS_BADREQUEST);
    LIBENCLOUD_DELETE(apiHandler);
    return ~0;
}

//
// private slots
//

void HttpHandler::_coreStateChanged (const QString &state)
{
    _coreState = state;
}

void HttpHandler::_needReceived (const QString &what)
{
    if (!_need.contains(what))
        _need += " " + what;
}

//
// private methods
//

// return new instance of specific api handler given version - delete when finished
HttpAbstractHandler *HttpHandler::_versionToApiHandler (ApiVersion version)
{
    switch (version)
    {
        case API_VERSION_1:
            return new ApiHandler1(this);

        // future extensions here
    }

    return NULL;
}

}  // namespace libencloud
