#include <common/common.h>
#include <common/config.h>
//#include <include/utils.h>
#include <setup/ece/ececommon.h>
#include <setup/ece/retrconfmsg.h>

namespace libencloud {

//
// public methods
//

const VpnConfig *RetrConfMsg::getVpnConfig () const
{
    return &_vpnConfig;
}

//
// public slots
//

int RetrConfMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QSslConfiguration config;
    QMap<QByteArray, QByteArray> headers;

    EMIT_ERROR_ERR_IF (_cfg == NULL);

    // client already destroyed via deleteLater()
    //LIBENCLOUD_DELETE (_client);
    LIBENCLOUD_ERR_IF ((_client = new Client) == NULL);

    EMIT_ERROR_ERR_IF (setupece::loadSslConfig(setupece::ProtocolTypeOp, _cfg, url, config));

    EMIT_ERROR_ERR_IF (_packRequest());
    EMIT_ERROR_ERR_IF (_encodeRequest(url, params));

    // setup signals from client
    connect(_client, SIGNAL(error(libencloud::Error)), this, SIGNAL(error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>)), this, SLOT(_clientComplete(QString)));

    _client->run(url, params, headers, config);

    return 0;
err:
    LIBENCLOUD_DELETE(_client);
    return ~0;
}

//
// private slots
// 
void RetrConfMsg::_clientComplete (const QString &response)
{
    EMIT_ERROR_ERR_IF (_decodeResponse(response));
    EMIT_ERROR_ERR_IF (_unpackResponse());

    emit processed();
err:
    sender()->deleteLater();
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

