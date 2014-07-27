#define LIBENCLOUD_DISABLE_TRACE
#include <QDir>
#include <QRegExp>
#include <encloud/Utils>
#include <encloud/Http/HttpHandler>
#include <common/common.h>
#include <common/config.h>
#include <http/handler.h>

namespace libencloud {

//
// public methods
//

HttpHandler::HttpHandler ()
{
    LIBENCLOUD_TRACE;

    _clear();
}

HttpHandler::~HttpHandler ()
{
    LIBENCLOUD_TRACE;
}

const Error &HttpHandler::getCoreError () const     { return _coreError; }
State HttpHandler::getCoreState () const            { return _coreState; }
Progress HttpHandler::getCoreProgress () const      { return _coreProgress; }
bool HttpHandler::getFallback () const              { return _isFallback; }
QString HttpHandler::getNeed () const               { return _needs.join(" "); }

void HttpHandler::removeNeed (const QString &what)
{
    LIBENCLOUD_TRACE;

    _needs.removeAll(what);
}

#ifdef LIBENCLOUD_MODE_ECE
const QUuid &HttpHandler::getPoi  () const             { return _poi; }
#endif

#ifdef LIBENCLOUD_MODE_ECE
int HttpHandler::setPoi (const QUuid &uuid)
{
    LIBENCLOUD_ERR_IF (uuid.isNull());

    _poi = uuid;

    return 0;
err:
    return ~0;
}
#endif

#ifdef LIBENCLOUD_MODE_SECE
int HttpHandler::setLicense (const QString &license)
{
    LIBENCLOUD_TRACE;

    QUuid uuid = QUuid(license);
    LIBENCLOUD_ERR_MSG_IF (uuid.isNull(), "invalid license received: "
            << license);

    removeNeed("license");
    emit licenseSend(uuid);

    return 0;
err:
    return ~0;
}
#endif

int HttpHandler::setClientPort (int port)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (port < 0 || port > 65536);

    emit clientPortSend(port);

    return 0;
err:
    return ~0;
}

int HttpHandler::setAuth (const Auth &auth)
{
    LIBENCLOUD_DBG(auth.toString());

    if (auth.isValid())
    {
        switch (auth.getId())
        {
            case Auth::SwitchboardId:
                removeNeed("sb_auth");
                break;
            case Auth::ProxyId:
                removeNeed("proxy_auth");
                break;
            default:
                LIBENCLOUD_ERR_IF (1);
        }

    } // otherwise can be a credential reset

    emit authSupplied(auth);

    return 0;
err:
    return ~0;
}

const QVariant &HttpHandler::getServerConfig () const
{
    return _serverConfig;
}

int HttpHandler::setAction (const QString &action, const Params &params)
{
    LIBENCLOUD_DBG("action: " << action << ", params: " << params);

    emit actionRequest(action, params);

    return 0;
}

int HttpHandler::setConfig (const QVariant &config)
{
    emit configSupplied(config);

    return 0;
}

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

//    LIBENCLOUD_DBG("path: " << path << ", version: " << version <<
//            ", action: " << action);

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

void HttpHandler::_coreErrorReceived (const libencloud::Error &err)
{
    _coreError = err;
}

void HttpHandler::_coreStateChanged (State state)
{
    LIBENCLOUD_TRACE;

    switch (state)
    {
        case libencloud::StateIdle:
            _clear();
            break;
        default:
            break;
    }

    _coreState = state;
}

void HttpHandler::_coreProgressReceived (const Progress &progress)
{
    LIBENCLOUD_TRACE;

    _coreProgress = progress;
}

void HttpHandler::_coreFallbackReceived (bool isFallback)
{
    LIBENCLOUD_DBG("isFallback: " << isFallback);

    _isFallback = isFallback;
}

void HttpHandler::_needReceived (const QString &what)
{
    LIBENCLOUD_DBG("what: " << what);

    if (!_needs.contains(what))
        _needs.append(what);
}

void HttpHandler::_serverConfigReceived (const QVariant &config)
{
    LIBENCLOUD_DBG("config: " << config);

    _serverConfig = config;
}

//
// private methods
//

void HttpHandler::_clear()
{
    _coreState = StateNone;
    _isFallback = false;
    _serverConfig.clear();
}

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
