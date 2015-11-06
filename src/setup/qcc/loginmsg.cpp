#define LIBENCLOUD_DISABLE_TRACE  // disable heave tracing
#include <QVariantMap>
#include <encloud/Utils>
#include <common/common.h>
#include <common/config.h>
#include <setup/qcc/loginmsg.h>
#if defined(LIBENCLOUD_MODE_QCC) && defined(LIBENCLOUD_SPLITDEPS)
#  include <common/qcc_version.h>
#endif

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
    QSslConfiguration sslconf;
    QSslCertificate cert;
    QVariantMap data = _data.toMap();
    QList<QSslCertificate> cas(cert.fromPath(_cfg->config.sslInit.caPath.absoluteFilePath()));

    if (!_sbAuth.isValid())
    {
        emit authRequired(Auth::SwitchboardId);
        LIBENCLOUD_EMIT_ERR (error(Error(tr("Switchboard login required"))));
    }

    EMIT_ERROR_ERR_IF (_cfg == NULL);

    // client already destroyed via deleteLater()
    //LIBENCLOUD_DELETE (_client);
    LIBENCLOUD_ERR_IF ((_client = new Client) == NULL);

    // Switchboard is strict on this
    headers["User-Agent"] = LIBENCLOUD_USERAGENT_QCC;

    // Initialization CA cert verification
    sslconf.setCaCertificates(cas);

    // Setup authentication data
    EMIT_ERROR_ERR_IF (crypto::configFromAuth(_sbAuth, url, headers, sslconf));

    if (data["in"].toBool())
    {
        params.addQueryItem("emi_username", _sbAuth.getUser());
        params.addQueryItem("emi_password", _sbAuth.getPass());
        url.setPath(LIBENCLOUD_SETUP_QCC_LOGIN_URL);
    }
    else
        url.setPath(LIBENCLOUD_SETUP_QCC_LOGOUT_URL);

    LIBENCLOUD_DBG("[Setup] Requesting login at URL: " << url.toString());

    // setup signals from client
    connect(_client, SIGNAL(error(libencloud::Error)), this, SIGNAL(error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    _client->setVerifyCA(_cfg->config.sslInit.verifyCA);
    _client->run(url, params, headers, sslconf);

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
    sender()->deleteLater();
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

