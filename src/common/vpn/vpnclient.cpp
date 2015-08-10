#include <QNetworkProxy>
#include <QSslCertificate>
#include <encloud/Logger>
#include <encloud/Utils>
#include <encloud/Vpn/VpnClient>
#include <encloud/Vpn/VpnConfig>
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
        case ProxyNotAllowed:
            return tr("Proxy not allowed");
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
            return "Idle";
        case StateConfigured:
            return "VPN configured";
        case StateStarting:
            return "About to launch process";
        case StateStarted:
            return "Process started";
        case StateConnecting:
            return "Connecting to VPN";
        case StateConnectingWaitingInitialResponse:
            return "Waiting for server response";
        case StateConnectingAuthenticating:
            return "Authenticating with server";
        case StateConnectingDownloadingConfiguration:
            return "Downloading VPN config";
        case StateConnectingAssigningIP:
            return "Assigning local IPs";
        case StateConnectingAddingRoutes:
            return "Adding routes";
        case StateConnected:
            return "VPN tunnel active";
        case StateExiting:
            return "VPN exiting";
    }
    return "";
}

void VpnClient::setState (VpnClient::State st)
{
    // only modify state if it's different
    if (st == state())
        return;

    LIBENCLOUD_DBG("[VPNClient] state: " << QString::number(st) <<
            " (" << stateString(st) << ")");

    LIBENCLOUD_RETURN_IF (!VpnClient::checkState(st), );

    emit stateChanged((this->st = st));
}

bool VpnClient::checkState (VpnClient::State state)
{
    return (state >= VpnClient::StateIdle && state <= VpnClient::StateLast);
}

// These values override configuration received from Switchboard
QStringList VpnClient::getArgs (const QString &vpnConfigPath)
{
    QStringList args;
    QString configPath;
    QString caCertPath;
    VpnConfig config;

    // we no longer log to file - messages are logged to encloud and forwarded to client
    //args << "--log" << _cfg->logPrefix + LIBENCLOUD_VPN_LOG_FILE;

    configPath = vpnConfigPath;
    args << "--config" << configPath;

    //
    // command-line extensions/overrides
    // 

    args << "--management" << LIBENCLOUD_VPN_MGMT_HOST << QString::number(_cfg->config.vpnMgmtPort);
    args << "--management-forget-disconnect";
    args << "--management-query-passwords";

    // in VPN mode, we jump to profile directory to make relative paths work
    if (!_cfg->config.setupEnabled)
        args << "--cd" << _cfg->config.vpnConfPath.absolutePath();

#ifndef LIBENCLOUD_MODE_VPN
    if (_cfg->config.setupEnabled)  // automatic setup via Switchboard -> CA is download
    {
        caCertPath = _cfg->config.sslOp.caPath.filePath();
        args << "--ca" << caCertPath;
    }  
    // else CA is user-specified
#endif

#ifdef Q_OS_WINCE // redirect output to log
    args << "--log" << "openvpnlog.log";
#endif

    if (sbAuth.isValid())
    {
        if (_cfg->config.sslOp.auth == LIBENCLOUD_AUTH_USERPASS)
        {
            args << "--auth-user-pass";
            args << "--auth-nocache";
        }
        else if (_cfg->config.sslOp.auth == LIBENCLOUD_AUTH_X509)
        {
            if (_cfg->config.sslOp.authFormat == LIBENCLOUD_AUTH_CERTKEY)
            {
                args << "--cert" << _cfg->config.sslOp.certPath.filePath();
                args << "--key" << _cfg->config.sslOp.keyPath.filePath();
            }
            else if (_cfg->config.sslOp.authFormat == LIBENCLOUD_AUTH_PKCS12)
            {
                args << "--pkcs12" << _cfg->config.sslOp.p12Path.filePath();
            }
        }
    } // otherwise we assume that authentication is file-based

    if (proxyAuth.isValid())
    {
        QUrl proxyUrl = proxyAuth.getUrl();

        switch (proxyAuth.getType())
        {
            case (Auth::SocksProxyType):
                args << "--socks-proxy" << proxyUrl.host() << QString::number(proxyUrl.port());
                break;
            case (Auth::HttpProxyType):
                args << "--http-proxy" << proxyUrl.host() << QString::number(proxyUrl.port());
                args << "auto";

                // mimic HTTP Proxy Type detection in 4iConnect 1X,2X
                if (proxyAuth.getUser().contains('\\'))
                    args << "ntlm";
                else
                    args << "basic";
                break;
            case (Auth::NoneType):
            default:
                break;
        }
    }

#ifdef Q_OS_WIN32
    args << "--dev-node" << LIBENCLOUD_TAPNAME;
#endif

    // fix routes not being applied properly on OSX in TAP mode
#ifdef Q_OS_MAC
    args << "--script-security" << "2";
    args << "--up" << getBinDir() + "/openvpn-up.sh";
    args << "--down" << getBinDir() + "/openvpn-down.sh";
    args << "--route-up" << getBinDir() + "/openvpn-route-up.sh";
    // avoid bad routes if brought up too early (enpoints unreachable)
    args << "--route-delay" << "2";
#endif

    // Note: '--remote' overrides breaks proxy (Assertion failed at proxy.c:217)
    // => handle them by substituting in configuration

    args << "--verb" << QString::number(_cfg->config.vpnVerbosity);

    foreach (QString arg, _cfg->config.vpnArgs.split(QRegExp("\\s+")))
        args << arg;

    //
    // consistency checks
    //
    LIBENCLOUD_EMIT_ERR_IF (config.fromFile(configPath, false),
            sigError(this->err = ConfigError));
#ifndef LIBENCLOUD_MODE_VPN
    if (proxyAuth.isValid())
    {
        LIBENCLOUD_EMIT_ERR_IF (
                config.getRemoteProto() == VpnConfig::UdpProto &&
                proxyAuth.getType() != Auth::NoneType,
                sigError(this->err = ProxyNotAllowed, "Server must be configured to use TCP"));
    }
#endif

    return args;
err:
    return QStringList();
}

void VpnClient::start (bool fallback)
{
    QStringList args;
    QString path;
    QString configPath;
    QFileInfo file;

    LIBENCLOUD_TRACE;

    if (this->process != NULL)
    {
        LIBENCLOUD_DBG("[VPNClient] error: already started");
        return;
    }

    LIBENCLOUD_EMIT_RETURN_MSG_IF ((this->st != StateIdle &&
                         this->st != StateConfigured),
            sigError(this->err = BadStateError),
            tr("Bad state: ") + QString::number(this->st), );

    LIBENCLOUD_NOTICE("[VPNClient] Starting");

    enableTap();

    if (fallback)
        configPath = _cfg->config.fallbackVpnConfPath.absoluteFilePath();
    else
        configPath = _cfg->config.vpnConfPath.absoluteFilePath();

    args = getArgs(configPath);
    if (args.empty())
    {
        LIBENCLOUD_DBG("[VPNClient] error: invalid arguments: " << configPath);
        return;
    }

    path = _cfg->config.vpnExePath.filePath();

    LIBENCLOUD_DBG("[VPNClient] fallback: " << fallback << ", path: " << path);

    file = QFileInfo(path);
    LIBENCLOUD_EMIT_ERR_IF (!file.isFile() || !file.isExecutable(),
            sigError(this->err = NotExecError, "path: " + path));

    this->process = new QProcess(this);
    LIBENCLOUD_EMIT_ERR_IF (this->process == NULL,
            sigError(this->err = MemoryError));

#ifndef Q_OS_WINCE
    this->process->setProcessChannelMode(QProcess::MergedChannels);
#endif

    connect(this->process, SIGNAL(error(QProcess::ProcessError)), this,
            SLOT(processError(QProcess::ProcessError)));
    connect(this->process, SIGNAL(started()), this,
            SLOT(processStarted()));
    connect(this->process, SIGNAL(readyRead()), this,
            SLOT(processReadyRead()));
    connect(this->process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(processFinished(int, QProcess::ExitStatus)));

    LIBENCLOUD_DBG("[VPNClient] Running " << QString(path +
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

    LIBENCLOUD_NOTICE("[VPNClient] Stopping");

    //LIBENCLOUD_DBG("[VPNClient] state: " << QString::number(this->process->state()));

#if defined Q_OS_UNIX && defined LIBENCLOUD_MODE_QCC
    // On MAC & Linux in Connect mode, OpenVPN runs from wrapper script...
    // avoid having to kill all children manually
    QString killWrapper = QString("pkill -P %1").arg(QString::number(this->process->pid()));
    LIBENCLOUD_DBG("[VPNClient] executing: " << killWrapper);
    int rc = QProcess::execute(killWrapper);
    LIBENCLOUD_DBG("[VPNClient] execute returned: " << rc);
#endif

    if (this->process->state() != QProcess::NotRunning)
    {
        this->process->blockSignals(true);
#ifdef Q_OS_WIN
        this->process->kill();
#else
#ifndef LIBENCLOUD_MODE_QCC
        // only if not already pkill-ed
        this->process->terminate();
#endif
#endif
        this->process->waitForFinished();
    }
    LIBENCLOUD_DELETE(this->process);

    disableTap();
    // assume we have already been configured, so skip the Idle state
    // don't emit the new state though to avoid automatic restart by Manager
    this->st = StateConfigured;
    this->err = NoError;
}

//
// public slots
//
void VpnClient::authSupplied (const Auth &auth)
{
    //LIBENCLOUD_ERR_IF (!auth.isValid());  // invalid objects used to perform reset

    switch (auth.getId())
    {
        case Auth::SwitchboardId:
            sbAuth = auth;
            break;
        case Auth::ProxyId:
            proxyAuth = auth;
            break;
        default:
            break;
    }
}

//
// private slots
//

void VpnClient::processError (QProcess::ProcessError err)
{
    QString errStr = this->process->errorString();

    LIBENCLOUD_DBG("[VPNClient] error: " << err << " (" << errStr << ")");

    emit sigError((this->err = ProcessError), errStr);

//    if (_cfg->config.vpnManualConfigPath.exists())
//        this->st = StateConfigured;
}

void VpnClient::processStarted ()
{
    LIBENCLOUD_TRACE;

    emit stateChanged((this->st = StateStarted));
}

void VpnClient::processReadyRead ()
{
    QProcess *process = qobject_cast<QProcess *> (sender());
    QString log = process->readAllStandardOutput() + process->readAllStandardError();

    foreach  (QString line, log.split(QRegExp("[\r\n]"), QString::SkipEmptyParts))
    {
        //qDebug() << "line: " << line;

        // remove leading timestamp
        QRegExp dateRE("^\\w{3} \\w{3} \\d{2} \\d{2}:\\d{2}:\\d{2} \\d{4} ");

        if (dateRE.indexIn(line) != -1)
            line.remove(0, dateRE.matchedLength());

        //qDebug() << "newline: " << line;
        
#ifdef LIBENCLOUD_MODE_QCC
        LIBENCLOUD_LOG(line.prepend("[VPN] "));
#else
        __LIBENCLOUD_SIMPLE_MSG(-1, "LOG", line.prepend("[VPN] "));
#endif
    }
}

void VpnClient::processFinished (int exitCode, QProcess::ExitStatus exitStatus)
{
    LIBENCLOUD_TRACE;

    // process exiting always indicates failure whatever the return code
    // for example upon bad cert authentication openvpn exits with status 0!
    LIBENCLOUD_EMIT_RETURN_MSG_IF (1, //exitCode || exitStatus,
            sigError(this->err = ProcessError),
            "Process exitCode: " + QString::number(exitCode) +
            ", exitStatus: " + QString::number(exitStatus), );

//    if (_cfg->vpnManualConfigPath.exists())
//        this->st = StateConfigured;
}

//
// private methods 
//

void VpnClient::enableTap()
{
    // tap device enabled by default on unix and embedded platforms
#ifdef Q_OS_WIN
    QString path = (qApp ? qApp->applicationDirPath() : QDir::currentPath()) + "/TapSetup.exe";
    QStringList args;
    QString out;

    args << "enable";

    LIBENCLOUD_ERR_IF (utils::execute(path, args, out));
err:
#endif  // Q_OS_WIN
    return;
}

void VpnClient::disableTap()
{
    // tap device enabled by default on unix and embedded platforms
#ifdef Q_OS_WIN
    QString path = (qApp ? qApp->applicationDirPath() : QDir::currentPath()) + "/TapSetup.exe";
    QStringList args;
    QString out;

    args << "disable";

    // wait = false to speed up disconnect [CONNECT-115]
    LIBENCLOUD_ERR_IF (utils::execute(path, args, out, false));
err:
#endif  // Q_OS_WIN
    return;
}

}  // namespace libencloud
