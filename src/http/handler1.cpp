#include <encloud/Http/HttpHandler>
#include <encloud/Json>
#include <common/common.h>
#include <common/config.h>
#include <common/utils.h>
#include <http/handler.h>
#include <http/handler1.h>

//
// defines
//

#define LIBENCLOUD_HANDLER_STATUS_PATH  "status"
#define LIBENCLOUD_HANDLER_AUTH_PATH    "auth"
#define LIBENCLOUD_HANDLER_SETUP_PATH   "setup"
#define LIBENCLOUD_HANDLER_CLOUD_PATH   "cloud"

// disable heavy tracing
#undef LIBENCLOUD_TRACE 
#define LIBENCLOUD_TRACE do {} while(0)

namespace libencloud {

//
// public methods
//

ApiHandler1::ApiHandler1 (HttpHandler *parent)
    : _parent(parent)
{
    LIBENCLOUD_TRACE;
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

    // jsonp handling
    //response.getHeaders()->set("Content-Type", "application/json");
    response.getHeaders()->set("Content-Type", "application/javascript");

    if (action == LIBENCLOUD_HANDLER_STATUS_PATH)
        return _handle_status(request, response);
    else if (action == LIBENCLOUD_HANDLER_AUTH_PATH)
        return _handle_auth(request, response);
    else if (action == LIBENCLOUD_HANDLER_SETUP_PATH)
        return _handle_setup(request, response);
    else if (action == LIBENCLOUD_HANDLER_CLOUD_PATH)
        return _handle_cloud(request, response);
    else 
        LIBENCLOUD_HANDLER_STATUS(LIBENCLOUD_HTTP_STATUS_NOTFOUND);

    return 0;
err:
    LIBENCLOUD_HANDLER_STATUS(LIBENCLOUD_HTTP_STATUS_BADREQUEST);
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
        {
            Progress progress = _parent->getCoreProgress();
            Error error = _parent->getCoreError();
            QVariantMap j;
            QVariantMap jMap;
            bool ok;

            j["state"] = _parent->getCoreState();

            // avoid "need" showing up as error
            if (_parent->getCoreState() == StateError &&
                error.isValid())
            {
                jMap["code"] = error.getCode();
                jMap["seq"] = error.getSeq();
                if (error.getDesc() != "")
                    jMap["desc"] = error.getDesc();
                if (error.getExtra() != "")
                    jMap["extra"] = error.getExtra();
                j["error"] = jMap;
            }

            if (progress.isValid())
            {
                jMap["desc"] = progress.getDesc();
                jMap["step"] = progress.getStep();
                jMap["total"] = progress.getTotal();
                j["progress"] = jMap;
            }

            if (_parent->getNeed() != "")
                j["need"] = _parent->getNeed();

            QString content = json::serialize(j, ok);
            LIBENCLOUD_HANDLER_ERR_IF (!ok, LIBENCLOUD_HTTP_STATUS_INTERNALERROR);

            response.setContent("jsonpCallback(" + content + ")");

            break;
        }
        default:
            LIBENCLOUD_HANDLER_ERR_IF (1, LIBENCLOUD_HTTP_STATUS_BADMETHOD);
    }

    // only reach here upon success
    LIBENCLOUD_HANDLER_OK;
    return 0;
err:
    return ~0;
}

int ApiHandler1::_handle_auth (const HttpRequest &request, HttpResponse &response)
{
    LIBENCLOUD_TRACE;

    switch (httpMethodFromString(request.getMethod()))
    {

#ifdef LIBENCLOUD_MODE_QIC
        case LIBENCLOUD_HTTP_METHOD_POST:
        {
            QUrl url;
            QString id, type, user, pass, aurl;

            LIBENCLOUD_HANDLER_ERR_IF (request.getHeaders()->get("Content-Type") !=
                        "application/x-www-form-urlencoded",
                    LIBENCLOUD_HTTP_STATUS_BADMETHOD);
            url.setEncodedQuery((*request.getContent()).toAscii());

            LIBENCLOUD_HANDLER_ERR_IF (
                    ((id = url.queryItemValue("id")) == ""),
                LIBENCLOUD_HTTP_STATUS_BADREQUEST);

            /* allow empty credentials for reset
            LIBENCLOUD_HANDLER_ERR_IF (
                    ((id = url.queryItemValue("id")) == "") ||
                    ((aurl = url.queryItemValue("url")) == "") ||
                    ((user = url.queryItemValue("user")) == "") ||
                    ((pass = url.queryItemValue("pass")) == ""),
                LIBENCLOUD_HTTP_STATUS_BADREQUEST);
                */

            type = url.queryItemValue("type");
            aurl = url.queryItemValue("url");
            user = url.queryItemValue("user");
            pass = url.queryItemValue("pass");

            Auth auth(id, type, aurl, user, pass);

            /* allow empty credentials for reset
            LIBENCLOUD_HANDLER_ERR_IF (!auth.isValid(),
                LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            */

            LIBENCLOUD_HANDLER_ERR_IF (_parent->setAuth(auth), 
                    LIBENCLOUD_HTTP_STATUS_INTERNALERROR);
        }
        break;
#endif
        case -1:  // make compiler happy
        default:
            LIBENCLOUD_HANDLER_ERR_IF (1, LIBENCLOUD_HTTP_STATUS_BADMETHOD);
    }

    // only reach here upon success
    LIBENCLOUD_HANDLER_OK;
    return 0;
err:
    return ~0;
}

int ApiHandler1::_handle_setup (const HttpRequest &request, HttpResponse &response)
{
    LIBENCLOUD_TRACE;

    switch (httpMethodFromString(request.getMethod()))
    {

#ifdef LIBENCLOUD_MODE_ECE
        case LIBENCLOUD_HTTP_METHOD_GET:
        {
            QVariantMap j;
            bool ok;
  
            j["poi"] = utils::uuid2String(_parent->getPoi());

            QString content = json::serialize(j, ok);
            LIBENCLOUD_HANDLER_ERR_IF (!ok, LIBENCLOUD_HTTP_STATUS_INTERNALERROR);

            response.setContent("jsonpCallback(" + content + ")");
  
            break;
        }
#endif

#ifdef LIBENCLOUD_MODE_SECE
        case LIBENCLOUD_HTTP_METHOD_POST:
        {
            QUrl url;
            QString val;

            LIBENCLOUD_HANDLER_ERR_IF (request.getHeaders()->get("Content-Type") !=
                        "application/x-www-form-urlencoded",
                    LIBENCLOUD_HTTP_STATUS_BADMETHOD);
            url.setEncodedQuery((*request.getContent()).toAscii());

            if ((val = url.queryItemValue("license")) != "")
            {
                LIBENCLOUD_HANDLER_ERR_IF (_parent->setLicense(val),
                        LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            }
            else
            {
                LIBENCLOUD_HANDLER_ERR_IF (1, LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            }
        }
        break;
#endif

#ifdef LIBENCLOUD_MODE_QIC
        case LIBENCLOUD_HTTP_METHOD_POST:
        {
            QUrl url;
            QString val;

            LIBENCLOUD_HANDLER_ERR_IF (request.getHeaders()->get("Content-Type") !=
                        "application/x-www-form-urlencoded",
                    LIBENCLOUD_HTTP_STATUS_BADMETHOD);
            url.setEncodedQuery((*request.getContent()).toAscii());

            if ((val = url.queryItemValue("clientPort")) != "")
            {
                LIBENCLOUD_HANDLER_ERR_IF (_parent->setClientPort(val.toInt()),
                        LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            }
            else
            {
                LIBENCLOUD_HANDLER_ERR_IF (1, LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            }

            break;
        }
#endif

#ifdef LIBENCLOUD_MODE_QIC
        case LIBENCLOUD_HTTP_METHOD_GET:
        {
            QVariant j = _parent->getServerConfig();
            bool ok;

            QString content = json::serialize(j, ok);
            LIBENCLOUD_HANDLER_ERR_IF (!ok, LIBENCLOUD_HTTP_STATUS_INTERNALERROR);

            response.setContent("jsonpCallback(" + content + ")");
  
            break;
        }
#endif

        case -1:  // make compiler happy
        default:
            LIBENCLOUD_HANDLER_ERR_IF (1, LIBENCLOUD_HTTP_STATUS_BADMETHOD);
    }

    // only reach here upon success
    LIBENCLOUD_HANDLER_OK;
    return 0;
err:
    return ~0;
}

int ApiHandler1::_handle_cloud (const HttpRequest &request, HttpResponse &response)
{
    LIBENCLOUD_TRACE;

    switch (httpMethodFromString(request.getMethod()))
    {
        case LIBENCLOUD_HTTP_METHOD_POST:
        {
            QUrl url;
            QString val;

            LIBENCLOUD_HANDLER_ERR_IF (request.getHeaders()->get("Content-Type") !=
                        "application/x-www-form-urlencoded",
                    LIBENCLOUD_HTTP_STATUS_BADMETHOD);
            url.setEncodedQuery((*request.getContent()).toAscii());

            if ((val = url.queryItemValue("action")) != "")
            {
                url.removeQueryItem("action");

                LIBENCLOUD_HANDLER_ERR_IF (_parent->setAction(val, url.queryItems()),
                        LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            }
            else
            {
                LIBENCLOUD_HANDLER_ERR_IF (1, LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            }

            break;
        }
        default:
            LIBENCLOUD_HANDLER_ERR_IF (1, LIBENCLOUD_HTTP_STATUS_BADMETHOD);
    }

    // only reach here upon success
    LIBENCLOUD_HANDLER_OK;
    return 0;
err:
    return ~0;
}

}  // namespace libencloud
