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

QString HttpHandler::getCoreError () const      { return _coreError; }
State HttpHandler::getCoreState () const        { return _coreState; }
Progress HttpHandler::getCoreProgress () const  { return _coreProgress; }
QString HttpHandler::getNeed () const           { return _need; }

#ifdef LIBENCLOUD_MODE_ECE
QUuid HttpHandler::getPoi  () const             { return _poi; }
#endif

#ifdef LIBENCLOUD_MODE_SECE
int HttpHandler::setLicense (const QString &license)
{
    LIBENCLOUD_TRACE;

    QUuid uuid = QUuid(license);
    LIBENCLOUD_ERR_MSG_IF (uuid.isNull(), "invalid license received: "
            << license);

    emit licenseSend(uuid);

    return 0;
err:
    return ~0;
}
#endif

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

    LIBENCLOUD_ERR_IF (path.indexOf(versionRx) < 0);

    version = versionRx.cap(1);
    action = versionRx.cap(2);

    LIBENCLOUD_DBG("path: " << path << ", version: " << version <<
            ", action: " << action);

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

void HttpHandler::_coreErrorReceived (const QString &msg)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DBG("msg: " << msg);

    _coreError = msg;
}

void HttpHandler::_coreStateChanged (State state)
{
    LIBENCLOUD_TRACE;

    _coreState = state;
}

void HttpHandler::_coreProgressReceived (const Progress &progress)
{
    LIBENCLOUD_TRACE;

    _coreProgress = progress;
}

void HttpHandler::_needReceived (const QString &what)
{
    LIBENCLOUD_TRACE;

    if (!_need.contains(what))
        _need += " " + what;
}

//
// private methods
//

// return new instance of specific api handler given version 
// delete when finished
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
