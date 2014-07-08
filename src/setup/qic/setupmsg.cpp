#define LIBENCLOUD_DISABLE_TRACE  // disable heave tracing
#include <QSslCertificate>
#include <QVariantMap>
#include <common/common.h>
#include <common/config.h>
#include <common/utils.h>
#include <setup/qic/setupmsg.h>

// use only to wrap upper-level methods, otherwise duplicates will be emitted
#define EMIT_ERROR_ERR_IF(cond) \
    LIBENCLOUD_EMIT_ERR_IF(cond, error(Error(Error::CodeGeneric)))

namespace libencloud {

//
// public methods
//

SetupMsg::SetupMsg ()
    : MessageInterface()
{
    clear();
}

int SetupMsg::clear ()
{
    LIBENCLOUD_TRACE;

    MessageInterface::clear();

    _sbAuth = Auth();
    _vpnConfig.clear();
    _fallbackVpnConfig.clear();
    _caCert.clear();
    _serverConfig.clear();

    return 0;
}

const VpnConfig *SetupMsg::getVpnConfig () const
{
    return &_vpnConfig;
}

const VpnConfig *SetupMsg::getFallbackVpnConfig () const
{
    return &_fallbackVpnConfig;
}

//
// public slots
//

int SetupMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration sslconf;
    QString authData;
    QString headerData;
    QSslCertificate cert;
    
    LIBENCLOUD_DBG("CA path: " << _cfg->config.sslInit.caPath.absoluteFilePath());
    QList<QSslCertificate> cas(cert.fromPath(_cfg->config.sslInit.caPath.absoluteFilePath()));

    if (!_sbAuth.isValid())
    {
        emit authRequired(Auth::SwitchboardId);
        LIBENCLOUD_EMIT_ERR (error(Error(tr("Switchboard login required"))));
    }

    authData = _sbAuth.getUser() + ":" + _sbAuth.getPass();
    headerData = "Basic " + QByteArray(authData.toLocal8Bit().toBase64());

    EMIT_ERROR_ERR_IF (_cfg == NULL);
    EMIT_ERROR_ERR_IF (_client == NULL);
    
    url.setUrl(_sbAuth.getUrl());
    url.setPath(LIBENCLOUD_SETUP_QIC_CONFIG_URL);

    LIBENCLOUD_DBG("url: " << url);

    // Switchboard is strict on this
    headers["User-Agent"] = LIBENCLOUD_USERAGENT_QIC;
    headers["Authorization"] =  headerData.toLocal8Bit();

    // Initialization CA cert verification
    sslconf.setCaCertificates(cas);

    // setup signals from client
    disconnect(_client, 0, this, 0);
    connect(_client, SIGNAL(error(libencloud::Error)), this, SIGNAL(error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    _client->setVerifyCA(_cfg->config.sslInit.verifyCA);
    _client->run(url, params, headers, sslconf);

    return 0;
err:
    return ~0;
}

void SetupMsg::authSupplied (const Auth &auth)
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
void SetupMsg::_clientComplete (const QString &response)
{
    LIBENCLOUD_TRACE;

    // stop listening to signals from client
    disconnect(_client, 0, this, 0);

    LIBENCLOUD_ERR_IF (_decodeResponse(response));
    LIBENCLOUD_ERR_IF (_unpackResponse());

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
    QVariantMap map;
    bool ok;

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

    // field validity check
    LIBENCLOUD_EMIT_ERR_IF (
            json["uuid"].isNull() ||
            json["vpn_server_ip"].isNull() ||
            json["openvpn_conf"].isNull() ||
            json["available_pages"].isNull() ||
            json["openvpn_cert"].isNull(),
            error(Error(tr("Error parsing SB configuration"))));

    // most fields not used locally (retrieved by QIC via API)
    // for now we're only interested in VPN configuration,
    _vpnConfig = VpnConfig(json["openvpn_conf"].toString(), _cfg);
    LIBENCLOUD_EMIT_ERR_IF (!_vpnConfig.isValid(),
            error(Error(tr("VPN Configuration from Switchboard not valid"))));

    if (!json["fallback_openvpn_conf"].isNull())
    {
        _fallbackVpnConfig = VpnConfig(json["fallback_openvpn_conf"].toString(), _cfg);
        LIBENCLOUD_EMIT_ERR_IF (!_fallbackVpnConfig.isValid(),
                error(Error(tr("Fallback VPN Configuration from Switchboard not valid"))));
    }

    // and CA certificate
    _caCert = QSslCertificate(json["openvpn_cert"].toString().toAscii());
    LIBENCLOUD_EMIT_ERR_IF (!_caCert.isValid(),
            error(Error(tr("CA Certificate from Switchboard not valid"))));

    // remapping to Encloud configuration
    _serverConfig["time"] = json["time"];

    map["uuid"] = json["uuid"];
    map["openvpn_internal_ip"] = json["openvpn_internal_ip"];
    if (!json["fallback_openvpn_internal_ip"].isNull())
        map["fallback_openvpn_internal_ip"] = json["fallback_openvpn_internal_ip"];
    map["available_pages"] = json["available_pages"];
    _serverConfig["server"] = map;

    map.clear();
    map["already_connected"] = json["already_connected"];
    _serverConfig["client"] = map;

    emit (serverConfigSupply(_serverConfig));

    return 0;
err:
    return ~0;
}

int SetupMsg::_unpackResponse ()
{
    QString cafn = _cfg->config.sslOp.caPath.absoluteFilePath();
    QFile caf(cafn);

    // save the Operation CA certificate to file
    LIBENCLOUD_ERR_IF (!utils::fileCreate(caf, QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (caf.write(_caCert.toPem()) == -1);
    caf.close();

    return 0;
err:
    emit (error(Error(Error::CodeSystemError, tr("Failed writing Operation CA: ") +
                    caf.errorString())));

    caf.close();
    return ~0;
}

} // namespace libencloud
