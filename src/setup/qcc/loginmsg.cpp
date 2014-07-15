#define LIBENCLOUD_DISABLE_TRACE  // disable heave tracing
#include <QVariantMap>
#include <common/common.h>
#include <common/config.h>
#include <common/utils.h>
#include <setup/qcc/loginmsg.h>

// use only to wrap upper-level methods, otherwise duplicates will be emitted
#define EMIT_ERROR_ERR_IF(cond) \
    LIBENCLOUD_EMIT_ERR_IF(cond, error(Error(Error::CodeGeneric)))

namespace libencloud {

//
// public slots
//

int LoginMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration config;
    QString authData;
    QString headerData;
    QVariantMap data = _data.toMap();

    EMIT_ERROR_ERR_IF (_cfg == NULL);
    LIBENCLOUD_DELETE (_client);
    LIBENCLOUD_ERR_IF ((_client = new Client) == NULL);

    if (!_sbAuth.isValid())
    {
        emit authRequired(Auth::SwitchboardId);
        LIBENCLOUD_EMIT_ERR (error(Error(tr("Switchboard login required"))));
    }

    authData = _sbAuth.getUser() + ":" + _sbAuth.getPass();
    headerData = "Basic " + QByteArray(authData.toLocal8Bit().toBase64());

    url.setUrl(_sbAuth.getUrl());

    if (data["in"].toBool())
    {
        params.addQueryItem("emi_username", _sbAuth.getUser());
        params.addQueryItem("emi_password", _sbAuth.getPass());
        url.setPath(LIBENCLOUD_SETUP_QCC_LOGIN_URL);
    }
    else
        url.setPath(LIBENCLOUD_SETUP_QCC_LOGOUT_URL);

    LIBENCLOUD_DBG("url: " << url);

    // Switchboard is strict on this
    headers["User-Agent"] = LIBENCLOUD_USERAGENT_QCC;
    headers["Authorization"] =  headerData.toLocal8Bit();

    // setup signals from client
    connect(_client, SIGNAL(error(libencloud::Error)), this, SIGNAL(error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    _client->run(url, params, headers, config);

    return 0;
err:
    LIBENCLOUD_DELETE(_client);
    return ~0;
}

void LoginMsg::authSupplied (const Auth &auth)
{
    LIBENCLOUD_DBG("user: " << auth.getUser());

    switch (auth.getId())
    {
        case Auth::SwitchboardId:
            _sbAuth = auth;
            break;
        default:
            // Qt Proxy is handled globally in core
            break;
    }
}

//
// private slots
// 
void LoginMsg::_clientComplete (const QString &response)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_decodeResponse(response));
    LIBENCLOUD_ERR_IF (_unpackResponse());

    emit processed();

err:
    _client->deleteLater();
    return;
}

//
// private methods
// 

int LoginMsg::_packRequest ()
{
    return 0;
}

int LoginMsg::_encodeRequest (QUrl &url, QUrl &params)
{
    LIBENCLOUD_UNUSED(url);
    LIBENCLOUD_UNUSED(params);

    return 0;
}

int LoginMsg::_decodeResponse (const QString &response)
{
    QVariantMap json;
    QVariantMap map;
    bool ok;

// XXX
    LIBENCLOUD_DBG("response: " << response);

    json = json::parse(response, ok).toMap();

    LIBENCLOUD_EMIT_ERR_IF (!ok,
            error(Error(tr("Error parsing SB response"))));

    // bubble Switchboard errors
    if (ok && !json["error"].isNull())
    {
        QString sbError = json["error"].toString();

        if (sbError == "Unauthorized")
        {
            emit authRequired(Auth::SwitchboardId);
            LIBENCLOUD_EMIT_ERR (error(Error(Error::CodeAuthFailed)));
        }
        else
        {
            LIBENCLOUD_EMIT_ERR (error(Error(Error::CodeServerError,
                        json["error"].toString())));
        }
    }

    return 0;
err:
    return ~0;
}

int LoginMsg::_unpackResponse ()
{
    return 0;
}

} // namespace libencloud

