#define LIBENCLOUD_DISABLE_TRACE  // disable heave tracing
#include <encloud/CookieJar>
#include <encloud/Utils>
#include <common/common.h>
#include <common/config.h>
//#if defined(LIBENCLOUD_MODE_QCC) && !defined(LIBENCLOUD_SPLITDEPS)
//#  include <common/qcc_version.h>
//#endif

// use only to wrap upper-level methods, otherwise duplicates will be emitted
#define EMIT_ERROR_ERR_IF(cond) \
    LIBENCLOUD_EMIT_ERR_IF(cond, error(Error(Error::CodeGeneric)))

#include <setup/common/closemsg.h>

namespace libencloud {

//
// public methods
//

CloseMsg::CloseMsg ()
    : MessageInterface()
    , _qnam(NULL)
{
    clear();
}

int CloseMsg::clear ()
{
    LIBENCLOUD_TRACE;

    MessageInterface::clear();

    _sbAuth = Auth();

    return 0;
}

int CloseMsg::setNetworkAccessManager (QNetworkAccessManager *qnam)
{
    LIBENCLOUD_RETURN_IF (qnam == NULL, ~0);

    _qnam = qnam;

    return 0;
}

//
// public slots
//

/* Errors are not critical - just log them */
int CloseMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration sslconf;
    
    LIBENCLOUD_RETURN_IF (_cfg == NULL, ~0);
    LIBENCLOUD_RETURN_IF (_qnam == NULL, ~0);

    LIBENCLOUD_DBG("[Setup] CA path: " << _cfg->config.sslInit.caPath.absoluteFilePath());

    LIBENCLOUD_RETURN_IF (!_sbAuth.isValid(), ~0);

    // Switchboard is strict on this
    headers["User-Agent"] = LIBENCLOUD_USERAGENT_QCC;

    LIBENCLOUD_DBG("[Setup] User Agent: " << headers["User-Agent"]);

    LIBENCLOUD_NOTICE("[Setup] Closing connection using Basic Auth");
    LIBENCLOUD_ERR_IF (crypto::configFromAuth(_sbAuth, url, headers, sslconf));

    LIBENCLOUD_ERR_IF ((_client = new Client) == NULL);
    LIBENCLOUD_ERR_IF ((_client->setNetworkAccessManager(_qnam)));

    // setup signals from client
    connect(_client, SIGNAL(error(libencloud::Error, QVariant)), this, SLOT(_error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>, QVariant)),
            this, SLOT(_clientComplete(QString, QMap<QByteArray, QByteArray>)));
    _client->setVerifyCA(false); // already verified in setupmsg

    url.setUrl(_sbAuth.getUrl());
    url.setPath(LIBENCLOUD_SETUP_QCC_CLOSE_URL);

    _client->run(url, params, headers, sslconf);

    return 0;
err:
    LIBENCLOUD_DELETE_LATER(_client);
    return ~0;
}

void CloseMsg::authSupplied (const libencloud::Auth &auth)
{
    LIBENCLOUD_DBG("user: " << auth.getUser());

    // ignore clear messages to make sure call succeeds
    if (!auth.isValid())
        return;

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

void CloseMsg::_error (const libencloud::Error &error)
{
    LIBENCLOUD_UNUSED(error);

    LIBENCLOUD_TRACE;

    emit processed();

    _sbAuth = Auth();
    LIBENCLOUD_DELETE_LATER (_client);
}

void CloseMsg::_clientComplete (const QString &response, const QMap<QByteArray, QByteArray> &headers)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_decodeResponse(response, headers));

    emit processed();

err:
    _sbAuth = Auth();
    LIBENCLOUD_DELETE_LATER (_client);
}

//
// private methods
// 

int CloseMsg::_packRequest ()
{
    return 0;
}

int CloseMsg::_encodeRequest (QUrl &url, QUrl &params)
{
    LIBENCLOUD_UNUSED(url);
    LIBENCLOUD_UNUSED(params);

    return 0;
}

int CloseMsg::_decodeResponse (const QString &response, const QMap<QByteArray, QByteArray> &headers)
{
    LIBENCLOUD_UNUSED(headers);

    LIBENCLOUD_DBG("[Setup] response: " << response);

    return 0;
}

} // namespace libencloud
