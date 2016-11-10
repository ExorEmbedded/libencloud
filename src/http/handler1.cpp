#define LIBENCLOUD_DISABLE_TRACE
#include <encloud/Api/CommonApi>
#include <encloud/Http/HttpHandler>
#include <encloud/Json>
#include <encloud/Utils>
#include <common/common.h>
#include <common/config.h>
#include <http/handler.h>
#include <http/handler1.h>

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
    LIBENCLOUD_ERR_IF (versionRx.captureCount() < 2);

    version = versionRx.cap(1);
    LIBENCLOUD_ERR_IF (version.toInt() != API_VERSION_1);

    action = versionRx.cap(2);

    //LIBENCLOUD_DBG("path: " << path);
    //LIBENCLOUD_DBG("action: " << action);

    if (action == LIBENCLOUD_API_STATUS)
        return _handle_status(request, response);
    else if (action == LIBENCLOUD_API_AUTH)
        return _handle_auth(request, response);
    else if (action == LIBENCLOUD_API_SETUP)
        return _handle_setup(request, response);
    else if (action == LIBENCLOUD_API_CLOUD)
        return _handle_cloud(request, response);
    else if (action == LIBENCLOUD_API_CONFIG)
        return _handle_config(request, response);
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
    QUrl url;

    LIBENCLOUD_TRACE;
    url.setEncodedQuery((*request.getContent()).toAscii());

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

            if (_parent->getCoreState() == StateError)
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

            if (_parent->getFallback())
                j["fallback"] = true;

            if (_parent->getNeed() != QVariant())
                j["need"] = _parent->getNeed();

            QString content = json::serialize(j, ok);
            LIBENCLOUD_HANDLER_ERR_IF (!ok, LIBENCLOUD_HTTP_STATUS_INTERNALERROR);
            _setContent(request, response, content);
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
    QUrl url;

    LIBENCLOUD_TRACE;
    url.setEncodedQuery((*request.getContent()).toAscii());

    switch (httpMethodFromString(request.getMethod()))
    {

#if defined(LIBENCLOUD_MODE_QCC) || defined(LIBENCLOUD_MODE_VPN)
        case LIBENCLOUD_HTTP_METHOD_POST:
        {
            QString id, type, user, pass, path, p12pass, aurl;

            LIBENCLOUD_HANDLER_ERR_IF (request.getHeaders()->get("Content-Type") !=
                        "application/x-www-form-urlencoded",
                    LIBENCLOUD_HTTP_STATUS_BADMETHOD);

            /* allow empty credentials for reset
            LIBENCLOUD_HANDLER_ERR_IF (
                    ((id = url.queryItemValue("id")) == "") ||
                    ((aurl = url.queryItemValue("url")) == "") ||
                    ((user = url.queryItemValue("user")) == "") ||
                    ((pass = url.queryItemValue("pass")) == ""),
                LIBENCLOUD_HTTP_STATUS_BADREQUEST);
                */

            id = url.queryItemValue("id");
            if (id.isEmpty())
                id = LIBENCLOUD_AUTH_SB;

            type = url.queryItemValue("type");
            if (type.isEmpty())
                type = LIBENCLOUD_AUTH_USERPASS;

            aurl = url.queryItemValue("url");
            user = url.queryItemValue("user");
            pass = url.queryItemValue("pass");
            path = url.queryItemValue("path");
            p12pass = url.queryItemValue("p12pass");

            Auth auth(id, type, aurl, user, pass, path, p12pass);

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
    QUrl url;

    LIBENCLOUD_TRACE;
    url.setEncodedQuery((*request.getContent()).toAscii());

    switch (httpMethodFromString(request.getMethod()))
    {

#if !defined(LIBENCLOUD_MODE_SECE)
        case LIBENCLOUD_HTTP_METHOD_GET:
        {
            bool ok;
#if defined(LIBENCLOUD_MODE_ECE)
            QVariantMap j;
            j["poi"] = utils::uuid2String(_parent->getPoi());
#elif defined(LIBENCLOUD_MODE_QCC)
            QVariant j = _parent->getServerConfig();
#elif defined(LIBENCLOUD_MODE_VPN)
            QVariant j;
#endif
            QString content = json::serialize(j, ok);
            LIBENCLOUD_HANDLER_ERR_IF (!ok, LIBENCLOUD_HTTP_STATUS_INTERNALERROR);

            _setContent(request, response, content);
            break;
        }
#endif

        case LIBENCLOUD_HTTP_METHOD_POST:
        {
            QString val;

            LIBENCLOUD_HANDLER_ERR_IF (request.getHeaders()->get("Content-Type") !=
                        "application/x-www-form-urlencoded",
                    LIBENCLOUD_HTTP_STATUS_BADMETHOD);

#if defined(LIBENCLOUD_MODE_SECE)
            if ((val = url.queryItemValue("license")) != "")
            {
                LIBENCLOUD_HANDLER_ERR_IF (_parent->setLicense(val),
                        LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            }
#elif defined(LIBENCLOUD_MODE_QCC)
            if ((val = url.queryItemValue("clientPort")) != "")
            {
                LIBENCLOUD_HANDLER_ERR_IF (_parent->setClientPort(val.toInt()),
                        LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            } 
            else if ((val = url.queryItemValue("logPort")) != "")
            {
                LIBENCLOUD_HANDLER_ERR_IF (_parent->setLogPort(val.toInt()),
                        LIBENCLOUD_HTTP_STATUS_BADREQUEST);
            }
#else
//defined(LIBENCLOUD_MODE_VPN) ||
//defined(LIBENCLOUD_MODE_ECE)
            if (0) {}
#endif
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

int ApiHandler1::_handle_cloud (const HttpRequest &request, HttpResponse &response)
{
    QUrl url;

    LIBENCLOUD_TRACE;
    url.setEncodedQuery((*request.getContent()).toAscii());

    switch (httpMethodFromString(request.getMethod()))
    {
        case LIBENCLOUD_HTTP_METHOD_POST:
        {
            QString val;

            LIBENCLOUD_HANDLER_ERR_IF (request.getHeaders()->get("Content-Type") !=
                        "application/x-www-form-urlencoded",
                    LIBENCLOUD_HTTP_STATUS_BADMETHOD);

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

int ApiHandler1::_handle_config (const HttpRequest &request, HttpResponse &response)
{
    LIBENCLOUD_TRACE;

    switch (httpMethodFromString(request.getMethod()))
    {
        case LIBENCLOUD_HTTP_METHOD_GET:
        {
            QVariant config = _parent->getConfig();
            bool ok;

            QString content = json::serialize(config, ok);
            LIBENCLOUD_HANDLER_ERR_IF (!ok, LIBENCLOUD_HTTP_STATUS_INTERNALERROR);
            _setContent(request, response, content);

            break;
        }
        case LIBENCLOUD_HTTP_METHOD_POST:
        {
            QString js;
            QVariant json;
            bool ok;

            LIBENCLOUD_HANDLER_ERR_IF (request.getHeaders()->get("Content-Type") !=
                        "application/json",
                    LIBENCLOUD_HTTP_STATUS_BADMETHOD);

    		js = (*request.getContent()).toAscii();
			LIBENCLOUD_DBG("[Http] js: " << js);

			json = json::parse(js, ok);
            LIBENCLOUD_HANDLER_ERR_IF (!ok, LIBENCLOUD_HTTP_STATUS_BADREQUEST);

			_parent->setConfig(json);

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

void ApiHandler1::_setContent (const HttpRequest &request, HttpResponse &response, const QString &content)
{
    QString jsonpCallback = QUrl(request.getUrl()).queryItemValue("callback");

    if (jsonpCallback != "")
    {
        response.getHeaders()->set("Content-Type", "application/javascript");
        response.setContent(jsonpCallback + "(" + content + ")");
    }
    else
    {
        response.getHeaders()->set("Content-Type", "application/json");
        response.setContent(content);
    }
}

}  // namespace libencloud
