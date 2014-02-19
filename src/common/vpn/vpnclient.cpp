#include <QNetworkProxy>
#include <QSslCertificate>
#include <common/vpn/vpnclient.h>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

VpnClient::VpnClient(const Config *config)
    : _cfg(config)
    , err(NoError)
    , st(StateIdle)
    , process(NULL)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_EMIT_RETURN_IF (config == NULL,
            sigError(this->err = BadParamsError), );

    // manual configuration
    //if (_cfg->config->vpnManualConfigPath.exists())
    //    emit stateChanged((this->st = StateConfigured));
}

VpnClient::~VpnClient()
{
    LIBENCLOUD_TRACE;

    stop();
}

//
// public methods
//

VpnClient::Error VpnClient::error ()
{
    return this->err;
}

QString VpnClient::errorString (Error err)
{
    switch (err)
    {
        case NoError:
            return tr("No error");
        case BadParamsError:
            return tr("Bad parameters");
        case MemoryError:
            return tr("Memory allocation failure");
        case BadStateError:
            return tr("Bad state");
        case ConfigError:
            return tr("Configuration error");
        case NotExecError:
            return tr("Not a valid executable");
        case ProcessError:
            return tr("Process execution error");
    }
    return "";
}

VpnClient::State VpnClient::state ()
{
    return this->st;
}

QString VpnClient::stateString (State st)
{
    switch (st)
    {
        case StateIdle:
            return "idle";
        case StateConfigured:
            return "vpn configured";
        case StateStarting:
            return "about to launch process";
        case StateStarted:
            return "process started";
        case StateConnecting:
            return "connecting to vpn";
        case StateConnectingWaitingInitialResponse:
            return "waiting for server response";
        case StateConnectingAuthenticating:
            return "authenticating with server";
        case StateConnectingDownloadingConfiguration:
            return "downloading vpn config";
        case StateConnectingAssigningIP:
            return "assigning local ips";
        case StateConnectingAddingRoutes:
            return "adding routes";
        case StateConnected:
            return "vpn tunnel active";
        case StateExiting:
            return "vpn exiting";
    }
    return "";
}

void VpnClient::setState (VpnClient::State st)
{
    // only modify state if it's different
    if (st == state())
        return;

    LIBENCLOUD_DBG("state: " << QString::number(st));

    LIBENCLOUD_RETURN_IF (!VpnClient::checkState(st), );

    emit stateChanged((this->st = st));
}

bool VpnClient::checkState (VpnClient::State state)
{
    return (state >= VpnClient::StateIdle && state <= VpnClient::StateLast);
}

// These values override configuration received from Switchboard
QStringList VpnClient::getArgs (void)
{
    QStringList args;
    QString configPath;
    QString caCertPath;
    QNetworkProxy proxy = QNetworkProxy::applicationProxy();

	configPath = _cfg->config.vpnConfPath.absoluteFilePath();

    args << "--config" << configPath;

    //
    // command-line extensions/overrides
    // 

    args << "--management" << LIBENCLOUD_VPN_MGMT_HOST << QString::number(_cfg->config.vpnMgmtPort);
    args << "--management-forget-disconnect";
    args << "--management-query-passwords";

#ifdef LIBENCLOUD_MODE_QIC
    args << "--auth-user-pass";
#endif

	caCertPath = _cfg->config.sslOp.caPath.absoluteFilePath();
    args << "--ca" << caCertPath;

    switch (proxy.type())
    {
        case (QNetworkProxy::Socks5Proxy):
            args << "--socks-proxy" << proxy.hostName() << QString::number(proxy.port());
            break;
        case (QNetworkProxy::HttpProxy):
            args << "--http-proxy" << proxy.hostName() << QString::number(proxy.port());
            args << "auto";
#ifdef Q_OS_WIN
            args << "ntlm";
#else
            args << "basic";
#endif
            break;
        case (QNetworkProxy::NoProxy):
        default:
            break;
    }

#ifdef Q_OS_WIN
    args << "--dev-node" << LIBENCLOUD_TAPNAME;
#endif

    // Note: '--remote' overrides breaks proxy (Assertion failed at proxy.c:217)
    // => handle them by substituting in configuration

    args << "--verb" << QString::number(_cfg->config.vpnVerbosity);

    foreach (QString arg, _cfg->config.vpnArgs.split(QRegExp("\\s+")))
        args << arg;

    return args;
}

void VpnClient::start (void)
{
    QStringList args;
    QString path;
    QFileInfo file;

    LIBENCLOUD_TRACE;

    if (this->process != NULL)
    {
        LIBENCLOUD_DBG("already started");
        return;
    }

    LIBENCLOUD_EMIT_RETURN_MSG_IF ((this->st != StateIdle &&
                         this->st != StateConfigured),
            sigError(this->err = BadStateError),
            tr("Bad state: ") + QString::number(this->st), );

    enableTap();

    args = getArgs();
    path = _cfg->config.vpnExePath.absoluteFilePath();

    file = QFileInfo(path);
    LIBENCLOUD_EMIT_ERR_IF (!file.isFile() || !file.isExecutable(),
            sigError(this->err = NotExecError, "path: " + path));

    this->process = new QProcess(this);
    LIBENCLOUD_EMIT_ERR_IF (this->process == NULL,
            sigError(this->err = MemoryError));

    this->process->setProcessChannelMode(QProcess::ForwardedChannels);

    connect(this->process, SIGNAL(error(QProcess::ProcessError)), this,
            SLOT(processError(QProcess::ProcessError)));
    connect(this->process, SIGNAL(started()), this,
            SLOT(processStarted()));
    connect(this->process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(processFinished(int, QProcess::ExitStatus)));

    LIBENCLOUD_DBG("starting " << QString(path +
            "  " + args.join(" ")));

    this->process->start(path, args);

    emit stateChanged((this->st = StateStarting));

    return;

err:
    LIBENCLOUD_DELETE(this->process);
    return;
}

void VpnClient::stop (void)
{
    if (this->st == StateIdle || 
            this->process == NULL)
        return;

    LIBENCLOUD_TRACE;

    LIBENCLOUD_DBG("state: " << QString::number(this->process->state()));

    if (this->process->state() != QProcess::NotRunning)
    {
        this->process->blockSignals(true);
#ifdef Q_OS_WIN32
        this->process->kill();
#else
        this->process->terminate();
        this->process->waitForFinished();
#endif
    }
    LIBENCLOUD_DELETE(this->process);

    disableTap();
    // assume we have already been configured, so skip the Idle state
    // don't emit the new state though to avoid automatic restart by Manager
    this->st = StateConfigured;
    this->err = NoError;
}

//
// private slots
//

#if 0
void VpnClient::gotLogin (const QString &url, const QString &user, const QString &pass)
{
    Q_UNUSED(user);
    Q_UNUSED(pass);

    this->remoteUrl = QUrl(url);
}

void VpnClient::sbGotConfig (QString sbConfig)
{
    QVariantMap json;
    bool ok;

    json = Json::parse(sbConfig, ok).toMap();

    LIBENCLOUD_EMIT_ERR_IF (!ok ||
            parseConfig(json) ||
            parseCaCert(json)
            , sigError(this->err = ConfigError));

    emit stateChanged((this->st = StateConfigured));
err:
    return;
}
#endif

void VpnClient::processError (QProcess::ProcessError err)
{
    QString errStr = this->process->errorString();

    LIBENCLOUD_DBG("err: " << err << " (" << errStr << ")");

    emit sigError((this->err = ProcessError), errStr);

//    if (_cfg->config.vpnManualConfigPath.exists())
//        this->st = StateConfigured;
}

void VpnClient::processStarted ()
{
    LIBENCLOUD_TRACE;

    emit stateChanged((this->st = StateStarted));
}

void VpnClient::processFinished (int exitCode, QProcess::ExitStatus exitStatus)
{
    LIBENCLOUD_EMIT_RETURN_MSG_IF (exitCode || exitStatus,
            sigError(this->err = ProcessError),
            "Process exitCode: " + QString::number(exitCode) +
            ", exitStatus: " + QString::number(exitStatus), );

    emit stateChanged((this->st = StateIdle));

//    if (_cfg->vpnManualConfigPath.exists())
//        this->st = StateConfigured;
}

//
// private methods 
//

void VpnClient::enableTap()
{
#ifdef Q_OS_WIN
    QString path = qgetenv("ProgramFiles") + "/" + LIBENCLOUD_INSTALLDIR + "/bin/TapSetup.exe";
    QStringList args;
    QString out;

    args << "enable";

    LIBENCLOUD_ERR_IF (utils::execute(path, args, out));
err:
#endif
    return;
}

void VpnClient::disableTap()
{
#ifdef Q_OS_WIN
    QString path = qgetenv("ProgramFiles") + "/" + LIBENCLOUD_INSTALLDIR + "/bin/TapSetup.exe";
    QStringList args;
    QString out;

    args << "disable";

    LIBENCLOUD_ERR_IF (utils::execute(path, args, out));
err:
#endif
    return;
}

#if 0
int VpnClient::parseConfig(const QVariantMap &jo)
{
    QString vpnConfig;
    QRegExp regexHostPort("\\nremote\\s+(\\S+)\\s+(\\d+)");

    LIBENCLOUD_ERR_IF (jo["openvpn_conf"].isNull());

    vpnConfig = jo["openvpn_conf"].toString() + "\n";
    regexHostPort.indexIn(vpnConfig);

    // Use host name instead of IP received from Switchboard - avoids problems
    // due to ip change (config not updated).
    // Note: '--remote' overrides breaks proxy (Assertion failed at proxy.c:217)
    // so we replace the configuration value instead of overriding
    LIBENCLOUD_ERR_IF (regexHostPort.capturedTexts().count() != 3);
    this->remotePort = regexHostPort.capturedTexts()[2].toInt();
    vpnConfig.replace(regexHostPort, "\nremote " +
            this->remoteUrl.host() + " " +
            QString::number(this->remotePort));

    // print out a one-line copy
    LIBENCLOUD_DBG("vpnConfig: " << QString(vpnConfig).replace("\n", "|"));

    LIBENCLOUD_ERR_IF (!_cfgFile.open() ||
            _cfgFile.write(vpnConfig.toAscii()) == -1);

    LIBENCLOUD_DBG("configFile: " << _cfgFile.fileName());
    _cfgFile.close();

    return 0;
err:
    return ~0;
}

int VpnClient::parseCaCert(const QVariantMap &jo)
{
    QSslCertificate caCert;

    LIBENCLOUD_ERR_IF (jo["openvpn_cert"].isNull());

    caCert = QSslCertificate(jo["openvpn_cert"].toString().toAscii());
    LIBENCLOUD_ERR_IF (!caCert.isValid());

    LIBENCLOUD_ERR_IF (!this->caCertFile.open() ||
            this->caCertFile.write(caCert.toPem()) == -1);

    LIBENCLOUD_DBG("caCertFile: " << this->caCertFile.fileName());
    this->caCertFile.close();

    return 0;
err:
    return ~0;
}
#endif

}  // namespace libencloud
