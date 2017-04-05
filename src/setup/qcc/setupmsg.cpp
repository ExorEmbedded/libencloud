#define LIBENCLOUD_DISABLE_TRACE  // disable heave tracing
#include <QVariantMap>
#include <encloud/Utils>
#include <common/common.h>
#include <common/config.h>
#include <setup/qcc/setupmsg.h>
#if defined(LIBENCLOUD_MODE_QCC) && !defined(LIBENCLOUD_SPLITDEPS)
#  include <common/qcc_version.h>
#endif

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
    QSslCertificate cert;
    
    LIBENCLOUD_RETURN_IF (_cfg == NULL, ~0);

    LIBENCLOUD_DBG("[Setup] CA path: " << _cfg->config.sslInit.caPath.absoluteFilePath());

    QList<QSslCertificate> cas(cert.fromPath(_cfg->config.sslInit.caPath.absoluteFilePath()));

    if (!_sbAuth.isValid())
    {
        emit authRequired(Auth::SwitchboardId, QVariant());
        LIBENCLOUD_EMIT_ERR (error(Error(tr("Switchboard login required"))));
    }

    // Switchboard is strict on this
    headers["User-Agent"] = LIBENCLOUD_USERAGENT_QCC;

    LIBENCLOUD_DBG("[Setup] User Agent: " << headers["User-Agent"]);

    // Initialization CA cert verification
    sslconf.setCaCertificates(cas);

    // Setup authentication data
    LIBENCLOUD_EMIT_ERR_IF (crypto::configFromAuth(_sbAuth, url, headers, sslconf),
            error(Error(Error::CodeBadCredentials)));

    url.setPath(LIBENCLOUD_SETUP_QCC_CONFIG_URL);

    LIBENCLOUD_NOTICE("[Setup] Requesting configuration from URL: " << url.toString());

    LIBENCLOUD_DELETE_LATER(_client);
    EMIT_ERROR_ERR_IF ((_client = new Client) == NULL);

    // setup signals from client
    connect(_client, SIGNAL(error(libencloud::Error)), this, SIGNAL(error(libencloud::Error)));
    connect(_client, SIGNAL(error(libencloud::Error)), this, SLOT(_error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>)),
            this, SLOT(_clientComplete(QString, QMap<QByteArray, QByteArray>)));

    _client->setVerifyCA(_cfg->config.sslInit.verifyCA);
    _client->run(url, params, headers, sslconf);

    return 0;
err:
    LIBENCLOUD_DELETE_LATER(_client);
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

void SetupMsg::_error (const libencloud::Error &error)
{
    LIBENCLOUD_UNUSED(error);

    LIBENCLOUD_TRACE;
}

void SetupMsg::_clientComplete (const QString &response, const QMap<QByteArray, QByteArray> &headers)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (_decodeResponse(response, headers));
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

int SetupMsg::_decodeResponse (const QString &response, const QMap<QByteArray, QByteArray> &headers)
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
            QStringList domains;

            if (headers.contains(LIBENCLOUD_SETUP_QCC_DOMAINS_HDR))
                domains = QString(headers[LIBENCLOUD_SETUP_QCC_DOMAINS_HDR]).split(LIBENCLOUD_SETUP_QCC_DOMAINS_SEPARATOR);

            // If domain header is specified and contains at least one item, a new authentication is required
            // (with user selection if there are two or more domains to choose from)
            if (domains.count() > 0)
            {
                emit authRequired(Auth::SwitchboardId, domains);
                LIBENCLOUD_EMIT_ERR (error(Error(Error::CodeAuthDomainRequired)));
            }
            else
            {
                emit authRequired(Auth::SwitchboardId, QVariant());
                LIBENCLOUD_EMIT_ERR (error(Error(Error::CodeAuthFailed)));
            }
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

    // most fields not used locally (retrieved by QCC via API)
    // for now we're only interested in VPN configuration,
    _vpnConfig = VpnConfig(json["openvpn_conf"].toString());
    LIBENCLOUD_EMIT_ERR_IF (!_vpnConfig.isValid(),
            error(Error(tr("VPN Configuration from Switchboard not valid"))));

    LIBENCLOUD_EMIT_ERR_IF (!json["already_connected"].isNull() && json["already_connected"].toBool(),
            error(Error(Error::CodeAuthAlreadyConnected)));

    if (!json["fallback_openvpn_conf"].isNull())
    {
        _fallbackVpnConfig = VpnConfig(json["fallback_openvpn_conf"].toString());
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

    emit (serverConfigSupply(_serverConfig));

    return 0;
err:
    return ~0;
}

int SetupMsg::_unpackResponse ()
{
    LIBENCLOUD_EMIT_RETURN_IF (_cfg == NULL, error(Error(Error::CodeGeneric)), ~0);

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
