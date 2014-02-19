#include <QVariantMap>
#include <common/common.h>
#include <common/config.h>
#include <common/utils.h>
#include <setup/qic/setupmsg.h>

// use only to wrap upper-level methods, otherwise duplicates will be emitted
#define EMIT_ERROR_ERR_IF(cond) LIBENCLOUD_EMIT_ERR_IF(cond, error())

namespace libencloud {

//
// public methods
//

const VpnConfig *SetupMsg::getVpnConfig ()
{
    return &_vpnConfig;
}

//
// public slots
//

void SetupMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration config;

    if (!_auth.isValid())
    {
        emit need ("sb_auth");
        return;
    }

    QString concatenated = _auth.getUser() + ":" + _auth.getPass();
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;

    EMIT_ERROR_ERR_IF (_cfg == NULL);
    EMIT_ERROR_ERR_IF (_client == NULL);

    url.setUrl(_auth.getUrl());
    url.setPath(LIBENCLOUD_SETUP_QIC_CONFIG_URL);

    LIBENCLOUD_DBG("url: " << url);

    // Switchboard is strict on this
    headers["User-Agent"] = LIBENCLOUD_USERAGENT_QIC;
    headers["Authorization"] =  headerData.toLocal8Bit();

    // setup signals from client
    disconnect(_client, 0, this, 0);
    connect(_client, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    _client->run(url, params, headers, config);

err:
    return;
}

// Only catch auth signals for SB
void SetupMsg::authSupplied (const Auth &auth)
{
    LIBENCLOUD_TRACE;

    if (auth.getType() != "sb")
        return;

    _auth = auth;

    process();
}

//
// private slots
// 
void SetupMsg::_clientComplete (const QString &response)
{
    LIBENCLOUD_TRACE;

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

int SetupMsg::_packRequest ()
{
    return 0;
}

int SetupMsg::_encodeRequest (QUrl &url, QUrl &params)
{
    LIBENCLOUD_UNUSED(url);
    LIBENCLOUD_UNUSED(params);

    return 0;
}

int SetupMsg::_decodeResponse (const QString &response)
{
    QVariantMap json;
    bool ok;

    json = json::parse(response, ok).toMap();

    // field validity check
    LIBENCLOUD_EMIT_ERR_IF (!ok ||
            json["uuid"].isNull() ||
            json["vpn_server_ip"].isNull() ||
            json["openvpn_conf"].isNull() ||
            json["available_pages"].isNull() ||
            json["openvpn_cert"].isNull(),
            error(tr("Error parsing SB configuration")));

    // most fields not used locally (retrieved by QIC via API)
    // for now we're only interested in VPN configuration,
    _vpnConfig = VpnConfig(json["openvpn_conf"].toString(), _cfg);
    LIBENCLOUD_EMIT_ERR_IF (!_vpnConfig.isValid(),
            error(tr("VPN Configuration from Switchboard not valid")));

    // and CA certificate
    _caCert = QSslCertificate(json["openvpn_cert"].toString().toAscii());
    LIBENCLOUD_EMIT_ERR_IF (!_caCert.isValid(),
            error(tr("CA Certificate from Switchboard not valid")));

    // TODO emit message with whole config for API

    return 0;
err:
    return ~0;
}

int SetupMsg::_unpackResponse ()
{
    QString cafn = _cfg->config.sslOp.caPath.absoluteFilePath();
    QFile caf(cafn);

    // save the Operation CA certificate to file
    LIBENCLOUD_EMIT_ERR_IF (!caf.open(QIODevice::WriteOnly) ||
                caf.write(_caCert.toPem()) == -1,
            error(tr("System error writing Operation CA to disk")));
    caf.close();

    return 0;
err:
    caf.close();
    return ~0;
}

} // namespace libencloud
