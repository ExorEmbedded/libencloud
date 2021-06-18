#include <common/common.h>
#include <common/config.h>
#include <encloud/Progress>
#include <encloud/Client>
#include <cloud/cloud.h>


#include <QNetworkInterface>

namespace libencloud {

//
// public methods
//

Cloud::Cloud (Config *cfg) 
    : _cfg(cfg)
    , _setup(NULL)
    , _isFallback(false)
    , _vpnClient(NULL)
    , _vpnManager(NULL)
    , _client(NULL)
{
    LIBENCLOUD_TRACE;

    _vpnClient = new VpnClient(_cfg);
    LIBENCLOUD_ERR_IF (_vpnClient == NULL);

    _vpnManager = new VpnManager(_cfg);
    LIBENCLOUD_ERR_IF (_vpnManager == NULL);

    // state change signals handled locally then routed
    connect(_vpnClient, SIGNAL(stateChanged(VpnClient::State)),
            this, SLOT(_vpnStateChanged(VpnClient::State)));

    // ip assignment and need requests (e.g. auth) routed from manager
    connect(_vpnManager, SIGNAL(ipAssigned(QString)),
            this, SIGNAL(ipAssigned(QString)));

    // local error handling
    connect(_vpnClient, SIGNAL(sigError(VpnClient::Error, QString)), 
            this, SLOT(_vpnClientErr(VpnClient::Error, QString)));
    connect(_vpnManager, SIGNAL(sigError(VpnManager::Error, QString)), 
            this, SLOT(_vpnManagerErr(VpnManager::Error, QString)));

    // authentication forwarding to client, to and from manager
    connect(_vpnManager, SIGNAL(authRequired(libencloud::Auth::Id)), 
            this, SIGNAL(authRequired(libencloud::Auth::Id)));
    connect(this, SIGNAL(authSupplied(libencloud::Auth)), 
            _vpnClient, SLOT(authSupplied(libencloud::Auth)));
    connect(this, SIGNAL(authSupplied(libencloud::Auth)), 
            _vpnManager, SLOT(authSupplied(libencloud::Auth)));

    // custom provisioning required
    connect(_vpnManager, SIGNAL(customProvisioningRequired(QString)),
            this, SLOT(_doCustomProvisioning(QString)));

    _retry.setBackoff(2);
    connect(&_retry, SIGNAL(timeout()), SLOT(_onRetry()));
err:
    return;
}

Cloud::~Cloud ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE(_vpnClient);
    LIBENCLOUD_DELETE(_vpnManager);
}

int Cloud::start (bool fallback) 
{
    LIBENCLOUD_TRACE;

    _isFallback = fallback;

    _vpnClient->start(fallback);

	return 0;
}

int Cloud::stop (bool retry) 
{
    LIBENCLOUD_TRACE;

    if (!retry)
        _retry.stop();

    _vpnManager->detach();
    _vpnClient->stop();

    // Flush firewall
    system("iptables -t nat -F ENCLOUD ");

	return 0;
}

int Cloud::getTotalSteps () const
{
    return VpnClient::StateLast - VpnClient::StateFirst + 1;
}

//
// private slots
//


void Cloud::_vpnStateChanged (VpnClient::State state)
{
    QString stateStr = VpnClient::stateString(state);

//    LIBENCLOUD_DBG("state: " << stateStr);

    switch (state)
    {
        case VpnClient::StateStarted:
            _vpnManager->attach(_vpnClient, LIBENCLOUD_VPN_MGMT_HOST,
                    _cfg->config.vpnMgmtPort);
            break;
        case VpnClient::StateConnecting:
        case VpnClient::StateConnectingWaitingInitialResponse:
        case VpnClient::StateConnectingAuthenticating:
        case VpnClient::StateConnectingDownloadingConfiguration:
        case VpnClient::StateConnectingAssigningIP:
        case VpnClient::StateConnectingAddingRoutes:
        case VpnClient::StateExiting:
            emit stateChanged(StateConnect);
            break;
        case VpnClient::StateConnected:
            emit stateChanged(StateCloud);
            emit fallback(_isFallback);
            break;
    }

    emit progress(Progress(stateStr, state, 
                VpnClient::StateLast - VpnClient::StateFirst + 1));
}

void Cloud::_vpnClientErr (VpnClient::Error err, const QString &errMsg)
{
    // If a valid fallback configuration has been received from
    // Switchboard and we haven't tried it yet, use it
#ifdef LIBENCLOUD_MODE_CLIENT
    if (!_isFallback && _setup && _setup->getFallbackVpnConfig() &&
            _setup->getFallbackVpnConfig()->isValid())
#else // fallback disabled
    if (0)
#endif
    {
        LIBENCLOUD_ERR("[Cloud] Retrying with fallback configuration");
        _restart(true, true);
    }
    else 
    {
        QString msg;

        msg = VpnClient::errorString(err);
        if (errMsg != "")
            msg += "\n\n" + errMsg;

        emit error(Error(Error::CodeClientFailure, msg));

        _restart(_isFallback);
    }
}

// manager socket failures are handled internally and timeout handled here -
// all other errors caught by _vpnClientErr()
void Cloud::_vpnManagerErr (VpnManager::Error err, const QString &errMsg)
{
    QString msg;

    switch (err)
    {
        case VpnManager::ConnTimeout:

            // If a valid fallback configuration has been received from
            // Switchboard and we haven't tried it yet, use it
#ifdef LIBENCLOUD_MODE_CLIENT
            if (!_isFallback && _setup && _setup->getFallbackVpnConfig() &&
                    _setup->getFallbackVpnConfig()->isValid())
#else // fallback disabled
            if (0)
#endif
            {
                LIBENCLOUD_ERR("[Cloud] Retrying with fallback configuration");
                _restart(true, true);
            }
            else
            {
                emit error(Error(Error::CodeClientTimeout));
                _restart();
            }
            break;

        default:
            msg = VpnManager::errorString(err);
            if (errMsg != "")
                msg += "\n\n" + errMsg;

            emit error(Error(Error::CodeClientFailure, msg));
            _restart();
            break;
    }
}

void Cloud::_onRetry ()
{
    LIBENCLOUD_TRACE;

    _vpnClient->start(_isFallback);
}

void Cloud::_doCustomProvisioning(const QString& ipStr)
{
#ifdef Q_OS_LINUX
    QHostAddress ip(ipStr);
    QString ifaceName;
    const QList<QNetworkInterface> ifs = QNetworkInterface::allInterfaces();
    for(int i=0; i<ifs.length() && ifaceName.length() == 0; i++) {
        QList<QNetworkAddressEntry> addrs = ifs[i].addressEntries();
        for(int j=0; j<addrs.length(); j++) {
            if ( addrs[j].ip() == ip ) {
                ifaceName = ifs[i].name();
                break;
            }
        }
    }
    
    if (ifaceName.length() <= 0) {
        LIBENCLOUD_ERR("[Cloud] failed to find network inteface matching vpn ip " << ipStr );
        return;
    }

    // find default gw for vpn to contact for getting configuration
    QFile f("/proc/net/route");
    if (!f.open(QIODevice::ReadOnly)) {
        LIBENCLOUD_ERR("[Cloud] failed to open /proc/net/route: " << f.errorString() );
        return;
    }
    
    QString line = f.readLine();
    QHostAddress gwAddr;
    while(line.length() > 0) {
        if (line.startsWith(ifaceName + "\t")) {
            QStringList data = line.split("\t");
            if (data.length() > 2) {
                QString gwIpStr = data[2];
                bool ok = false;
                uint gwIpStrV4 = gwIpStr.toUInt(&ok, 16);
                if (ok && gwIpStrV4 != 0) {
                    gwAddr =  QHostAddress(qToBigEndian(gwIpStrV4));
                    break;
                }
            }
        }
        line = f.readLine();
    }
    
    if (gwAddr.isNull()) {
        LIBENCLOUD_ERR("[Cloud] failed to vpn gateway address" );
        return;
    }
    
    QUrl url;
    url.setScheme("http");
    url.setHost(gwAddr.toString());
    url.setPath("/manage/commands/commands.access.internal.getGatewayConfig");
    QMap<QByteArray, QByteArray> hdrs;
    hdrs["user-agent"] = "Endian 4i Connect (Linux)";


    LIBENCLOUD_DELETE_LATER(_client);
    LIBENCLOUD_ERR_IF ( (_client = new Client()) == NULL);

    connect(_client, SIGNAL(error(libencloud::Error, QVariant)),
            this, SLOT(_customProvisioningError(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>, QVariant)),
            this, SLOT(_customProvisioningConfig(QString)));

    _client->run(url, url, hdrs, QSslConfiguration::defaultConfiguration());
    
#else
    LIBENCLOUD_ERR("[Cloud] _doCustomProvisioning is not supported in this OS");
#endif
err:
    ;
}

QString extractYamlValue(const QString &input, int pos) {
    int colonPos = input.indexOf(':', pos);
    int eolPos = input.indexOf('\n', colonPos);
    QString trimmed = input.mid(colonPos+1, eolPos-colonPos-1).trimmed();
    if ( trimmed.size() > 0 && (trimmed[0] == '\'' || trimmed[0] == '"')  && (trimmed[trimmed.length()-1] == '\'' || trimmed[trimmed.length()-1] == '"')) {
        return trimmed.mid(1, trimmed.length()-2);
    }
    return trimmed;
}

void Cloud::_customProvisioningConfig(const QString &config)
{
    // Flush current rules
    system("iptables -t nat -N ENCLOUD");
    system("iptables -t nat -F ENCLOUD");
    while(0 == system("iptables -t nat -D PREROUTING -j ENCLOUD")){}
    system("iptables  -t nat -A PREROUTING -j ENCLOUD ");
    
    
    // Exmaples:
    //        DNAT:
    //        - DST_DEV: VPN:ANY
    //          DST_IP: ''
    //          DST_PORT: 10240
    //          ENABLED: 'on'
    //          FILTER_TARGET: ALLOW
    //          LOG: 'off'
    //          NAT_TARGET: DNAT
    //          PROTO: tcp
    //          REMARK: ''
    //          SRC_DEV: VPN:virtualswitchboard&VPN:vpnswitchboard&VPN:switchboard
    //          SRC_IP: ''
    //          TARGET_IP: 10.2.3.4
    //          TARGET_PORT: '80'
    //        SNAT: []
    // todo: replace with yaml parsing
    int dnatPos = config.indexOf("DNAT:");
    int endPos = config.indexOf(QRegExp("\n\s*[^- ]"), dnatPos);
    QRegExp elRx("\n\s*-");
    int nextElementPos = config.indexOf(elRx, dnatPos);
    while (nextElementPos != -1 && (endPos < 0 || nextElementPos < endPos) ) {
        int enabledPos = config.indexOf("ENABLED", nextElementPos);
        if (extractYamlValue(config, enabledPos).contains("on")) {
            int dstPortPos = config.indexOf("DST_PORT", nextElementPos);
            int targetIpPos = config.indexOf("TARGET_IP", nextElementPos);
            int targetPortPos = config.indexOf("TARGET_PORT", nextElementPos);
            int protPos = config.indexOf("PROTO", nextElementPos);

            int dstPort = extractYamlValue(config, dstPortPos).toUInt();
            int targetPort = extractYamlValue(config, targetPortPos).toUInt();
            QString targetIp = extractYamlValue(config, targetIpPos);
            QStringList protocols = extractYamlValue(config, protPos).split(",");
            for(int p=0; p<protocols.size(); p++) {
                QString cmd = QString("iptables  -t nat -A ENCLOUD -j DNAT -p %4 --dport %1 --to %2:%3 ").arg(dstPort).arg(targetIp).arg(targetPort).arg(protocols.at(p));
                system(cmd.toUtf8().constData());
            }
//            QProcess::execute(cmd);
        } else {
            // skip this element since not enabled
        }
        int newNextElementPos = config.indexOf(elRx, nextElementPos+2);
        if (newNextElementPos != -1 && newNextElementPos <= nextElementPos) {
            LIBENCLOUD_DBG("[Cloud] Detected loop while parsing " << newNextElementPos << nextElementPos << " .. abort loop!" );
            break;
        }
        nextElementPos = newNextElementPos;
    }
}

void Cloud::_customProvisioningError(const Error &error)
{
    LIBENCLOUD_ERR("[Cloud] error retrieving provisioning data " << error.toString() );
}

//
// private methods
//

// *ECE always retries automatically, QCC does only if forced
void Cloud::_restart (bool fallback, bool force)
{
//    LIBENCLOUD_DBG("fallback: " << fallback << ", force: " << force);

    _isFallback = fallback;

    if (_cfg->config.autoretry || _cfg->config.setupAgent)
        force = true;

    if (force)
    {
        stop(true);
        _retry.start();
    }
    
    // QCC core does full stop()
}

} // namespace libencloud
