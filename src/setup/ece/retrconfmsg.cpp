#include <common/common.h>
#include <common/config.h>
#include <common/utils.h>
#include <setup/ece/ececommon.h>
#include <setup/ece/retrconfmsg.h>

namespace libencloud {

//
// public methods
//

const VpnConfig *RetrConfMsg::getVpnConfig ()
{
    return &_vpnConfig;
}

//
// public slots
//

void RetrConfMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QSslConfiguration config;
    QMap<QByteArray, QByteArray> headers;

    EMIT_ERROR_ERR_IF (_cfg == NULL);
    EMIT_ERROR_ERR_IF (_client == NULL);

    EMIT_ERROR_ERR_IF (setupece::loadSslConfig(setupece::ProtocolTypeOp, _cfg, url, config));

    EMIT_ERROR_ERR_IF (_packRequest());
    EMIT_ERROR_ERR_IF (_encodeRequest(url, params));

    // setup signals from client
    disconnect(_client, 0, this, 0);
    connect(_client, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    // for SNI, Host field in header must match name of operation-host
    headers["Host"] = LIBENCLOUD_GETCONFIG_HOSTNAME;

    _client->run(url, params, headers, config);

err:
    return;
}

//
// private slots
// 
void RetrConfMsg::_clientComplete (const QString &response)
{
    // stop listening to signals from client
    disconnect(_client, 0, this, 0);

    EMIT_ERROR_ERR_IF (_decodeResponse(response));
    EMIT_ERROR_ERR_IF (_unpackResponse());

    emit processed();
err:
    return;
}

//
// private methods
// 

int RetrConfMsg::_packRequest ()
{
    return 0;
}

int RetrConfMsg::_encodeRequest (QUrl &url, QUrl &params)
{
    LIBENCLOUD_UNUSED(params);

    url.setPath(LIBENCLOUD_CMD_GETCONFIG);

    return 0;
}

int RetrConfMsg::_decodeResponse (const QString &response)
{
    bool ok;
    QString errString;

    QVariantMap jo = json::parse(response, ok).toMap();
    LIBENCLOUD_ERR_IF (!ok || jo.isEmpty());

    // <TEST> failure
#if 0
    LIBENCLOUD_ERR_IF (1);
#endif

    errString = jo["error"].toString();
    if (!errString.isEmpty())
    {
        LIBENCLOUD_DBG ("SB error: " << errString);
        goto err;
    }

    jo = jo["vpn"].toMap();
    LIBENCLOUD_ERR_IF (jo.isEmpty());

    _time = utils::pytime2DateTime(jo["time"].toString());
    LIBENCLOUD_ERR_IF (!_time.isValid());

    _vpnConfig = VpnConfig(jo, _cfg);
    LIBENCLOUD_ERR_IF (!_vpnConfig.isValid());

    return 0;
err:
    return ~0;
}

int RetrConfMsg::_unpackResponse ()
{
    LIBENCLOUD_DBG(_vpnConfig.toString());

    return 0;
}

} // namespace libencloud

