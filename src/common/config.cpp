#include <encloud/Utils>
#include <common/config.h>
#include <common/helpers.h>

namespace libencloud {

//
// public methods
//

/* Set defaults from defaults.h definitions */
Config::Config ()
    : settings(NULL)
    , sysSettings(NULL)
{
    QString sep = "/";

#ifdef Q_OS_WIN32  // relative paths
    QString progFiles = QString(qgetenv("ProgramFiles"));

    // main prefix and binaries are product-specific (e.g. ConnectApp)
    prefix = progFiles + sep + QString(LIBENCLOUD_PRODUCTDIR);
    sbinPrefix = prefix + sep + LIBENCLOUD_SBIN_PREFIX;

    // configuration is package-specific (e.g. ConnectApp\libencloud) under %ProgramFiles%
    confPrefix = progFiles + sep + QString(LIBENCLOUD_INSTALLDIR) +
            sep + LIBENCLOUD_ETC_PREFIX;

#else  // Linux - absolute paths
    prefix = LIBENCLOUD_PREFIX_PATH;
    confPrefix = LIBENCLOUD_ETC_PREFIX;
    sbinPrefix = LIBENCLOUD_SBIN_PREFIX;
#endif

    dataPrefix = getCommonAppDataDir();
    logPrefix = getCommonLogDir();

    filePath = _joinPaths(confPrefix, QString(LIBENCLOUD_CONF_FILE));

    // data are package-specific on win

    settings = new QSettings(LIBENCLOUD_ORG, LIBENCLOUD_APP);
    LIBENCLOUD_ERR_IF (settings == NULL);

    sysSettings = new QSettings(QSettings::SystemScope, LIBENCLOUD_ORG, LIBENCLOUD_APP);
    LIBENCLOUD_ERR_IF (sysSettings == NULL);

err:
    return;
}

Config::~Config()
{
    LIBENCLOUD_DELETE(settings);
    LIBENCLOUD_DELETE(sysSettings);
}

QVariant Config::getVariant ()
{
    return _json;
}

QString Config::dump ()
{
    QString s;
    QTextStream ts(&s);
    bool ok;

    ts << "dumping configuration:";
    ts << endl;

#if defined(LIBENCLOUD_MODE_QCC)
    ts << "mode=QCC" << endl;
#elif defined(LIBENCLOUD_MODE_ECE)
    ts << "mode=ECE" << endl;
#elif defined(LIBENCLOUD_MODE_SECE)
    ts << "mode=SECE" << endl;
#endif
    ts << "prefix=" << prefix << endl;
    ts << "confPrefix=" << confPrefix << endl;
    ts << "sbinPrefix=" << sbinPrefix << endl;
    ts << "dataPrefix=" << dataPrefix << endl;
    ts << "logPrefix=" << logPrefix << endl;
    ts << "settings=" << settings->fileName() << endl;
    ts << "sysSettings=" << sysSettings->fileName() << endl;

    ts << libencloud::json::serialize(_json, ok);

    return s;
}

/* Read config from file */
int Config::loadFromFile ()
{
    bool ok;

    _json = libencloud::json::parseFromFile(filePath.absoluteFilePath(), ok);
    if (!ok || _json.isNull())
    {
        LIBENCLOUD_DBG("[Config] Failed parsing config file: " << filePath.absoluteFilePath());
        goto err;
    }
    _jsonOrig = _json;

    LIBENCLOUD_ERR_IF (_parse(_json.toMap()));

    return 0;
err: 
    return ~0;
}

//
// protected slots
//

void Config::receive (const QVariant &cfg)
{
    //qDebug() << "cfg: " << cfg;

    // reset requested
    if (cfg.toMap().isEmpty())
    {
        _json = _jsonOrig;
    }
    else
    {
        utils::variantMerge(_json, cfg);
    }

    //LIBENCLOUD_DBG("[Config] new cfg: " << dump());

    LIBENCLOUD_ERR_IF (_parse(_json.toMap()));

err:
    return;
}

//
// protected methods
//

int Config::_parse (const QVariantMap &jo)
{
    if (jo["bind"].isNull())
        config.bind = "";  // binds only to localhost and tap
    else
        config.bind = jo["bind"].toString();

#ifdef LIBENCLOUD_MODE_ECE
    if (jo["poi"].isNull())
        config.poiPath = _joinPaths(dataPrefix, LIBENCLOUD_POI_FILE);
    else
        config.poiPath = _joinPaths(dataPrefix, \
                jo["poi"].toString());
#endif

    if (jo["timeout"].isNull())
        config.timeout = LIBENCLOUD_TIMEOUT;
    else
        config.timeout = jo["timeout"].toInt();

    if (jo["csr"].toMap()["tmpl"].isNull())
        config.csrTmplPath = _joinPaths(dataPrefix, LIBENCLOUD_CSRTMPL_FILE);
    else
        config.csrTmplPath = _joinPaths(dataPrefix, \
                jo["csr"].toMap()["tmpl"].toString());

    LIBENCLOUD_ERR_IF (_parseSb(jo));
    LIBENCLOUD_ERR_IF (_parseSsl(jo, config.ssl));

    if (jo["rsa"].toMap()["bits"].isNull())
        config.rsaBits = LIBENCLOUD_RSA_BITS;
    else
    {
        config.rsaBits = jo["rsa"].toMap()["bits"].toInt();
        LIBENCLOUD_ERR_MSG_IF ((config.rsaBits == 0 || (config.rsaBits % 512) != 0),
                "rsa bits must be a multiple of 512!");
    }

    LIBENCLOUD_ERR_IF (_parseVpn(jo));
    LIBENCLOUD_ERR_IF (_parseLog(jo));

    return 0;
err:
    return ~0;
}

int Config::_parseSb (const QVariantMap &jo)
{
    config.sbUrl = QUrl(LIBENCLOUD_SB_URL);

    if (jo["sb"].isValid())
    {
        QVariantMap jot = jo["sb"].toMap();

        if (jot["url"].isValid())
            config.sbUrl = jot["url"].toString();
    }

    return 0;
}

int Config::_parseSsl (const QVariantMap &jo, libencloud_config_ssl_t &sc)
{
    QVariantMap jot;

    if (&sc == &config.ssl)
    {
        sc.sbUrl = QUrl(LIBENCLOUD_SB_URL);
        sc.auth = LIBENCLOUD_AUTH_USERPASS;
        sc.authFormat = LIBENCLOUD_AUTH_CERTKEY;
        sc.verifyCA = true;
        sc.caPath = _joinPaths(dataPrefix, LIBENCLOUD_CA_FILE);
        sc.certPath = _joinPaths(dataPrefix, LIBENCLOUD_CERT_FILE);
        sc.keyPath = _joinPaths(dataPrefix, LIBENCLOUD_KEY_FILE);
        sc.p12Path = _joinPaths(dataPrefix, LIBENCLOUD_P12_FILE);

        jot = jo["ssl"].toMap();
    }
    else if (&sc == &config.sslInit)
    {
        sc = config.ssl;
        sc.caPath = _joinPaths(dataPrefix, LIBENCLOUD_INIT_CA_FILE);
        sc.certPath = _joinPaths(dataPrefix, LIBENCLOUD_INIT_CERT_FILE);
        sc.keyPath = _joinPaths(dataPrefix, LIBENCLOUD_INIT_KEY_FILE);
        sc.p12Path = _joinPaths(dataPrefix, LIBENCLOUD_INIT_P12_FILE);

        jot = jo["init"].toMap();
    }
    else if (&sc == &config.sslOp)
    {
        sc = config.ssl;
        sc.caPath = _joinPaths(dataPrefix, LIBENCLOUD_OP_CA_FILE);
        sc.certPath = _joinPaths(dataPrefix, LIBENCLOUD_OP_CERT_FILE);
        sc.keyPath = _joinPaths(dataPrefix, LIBENCLOUD_OP_KEY_FILE);
        sc.p12Path = _joinPaths(dataPrefix, LIBENCLOUD_OP_P12_FILE);

        jot = jo["op"].toMap();
    }

    if (jot["auth"].isValid())
    {
        sc.auth = jot["auth"].toString();
        LIBENCLOUD_ERR_IF (sc.auth != LIBENCLOUD_AUTH_USERPASS &&
                sc.auth != LIBENCLOUD_AUTH_X509);
    }

    if (jot["auth_format"].isValid())
    {
        sc.authFormat = jot["auth_format"].toString();
        LIBENCLOUD_ERR_IF (sc.authFormat != LIBENCLOUD_AUTH_CERTKEY &&
                sc.authFormat != LIBENCLOUD_AUTH_PKCS12);
    }

    if (jot["verify_ca"].isValid())
        sc.verifyCA = jot["verify_ca"].toBool();

    if (jot["ca"].isValid())
        sc.caPath = _joinPaths(dataPrefix, jot["ca"].toString());

    if (jot["cert"].isValid())
        sc.certPath = _joinPaths(dataPrefix, jot["cert"].toString());

    if (jot["key"].isValid())
        sc.keyPath = _joinPaths(dataPrefix, jot["key"].toString());

    if (jot["p12"].isValid())
        sc.p12Path = _joinPaths(dataPrefix, jot["p12"].toString());

    sc.sbUrl = jot["sb"].toMap()["url"].toString();
    if (sc.sbUrl.isEmpty())
        sc.sbUrl = config.sbUrl;
    LIBENCLOUD_ERR_MSG_IF (sc.sbUrl.isEmpty(),
            "sb url undefined!");

    if (&sc == &config.ssl)
    {
        LIBENCLOUD_ERR_IF (_parseSsl(jot, config.sslInit));
        LIBENCLOUD_ERR_IF (_parseSsl(jot, config.sslOp));
    }
   
    return 0;
err:
    return ~0;
}

int Config::_parseVpn (const QVariantMap &jo)
{
    config.vpnExePath = _joinPaths(sbinPrefix, LIBENCLOUD_VPN_EXE_FILE);
    config.vpnConfPath = _joinPaths(dataPrefix, LIBENCLOUD_VPN_CONF_FILE);
    config.fallbackVpnConfPath = _joinPaths(dataPrefix, LIBENCLOUD_VPN_FALLBACK_CONF_FILE);
    config.vpnMgmtPort = LIBENCLOUD_VPN_MGMT_PORT;
    config.vpnVerbosity = LIBENCLOUD_VPN_VERBOSITY;
    config.vpnArgs = "";

    if (jo["vpn"].isNull())
        return 0;

    QVariantMap jot =  jo["vpn"].toMap();

    if (jot["path"].isValid())
        config.vpnExePath = _joinPaths(sbinPrefix, jot["path"].toString());

    if (jot["conf"].isValid())
        config.vpnConfPath = _joinPaths(dataPrefix, jot["conf"].toString());

    if (jot["fallback_conf"].isValid())
        config.fallbackVpnConfPath = _joinPaths(dataPrefix, jot["fallback_conf"].toString());

    if (jot["mgmt"].toMap()["port"].isValid())
        config.vpnMgmtPort = jot["mgmt"].toMap()["port"].toInt();

    if (jot["verb"].isValid())
        config.vpnVerbosity = jot["verb"].toInt();

    if (jot["args"].isValid())
        config.vpnArgs = jot["args"].toString();

    return 0;
}

int Config::_parseLog (const QVariantMap &jo)
{
    config.logLevel = LIBENCLOUD_LOG_LEV;

    if (jo["log"].isValid()) 
    {
        QVariantMap jot = jo["log"].toMap();

        if (jot["lev"].isValid())
        {
            config.logLevel = jot["lev"].toInt();
            LIBENCLOUD_ERR_MSG_IF ((config.logLevel < 0 || config.logLevel > 7),
                    "log level must be between 0 and 7!");
        }
    }

    // set this value globally for log macros
    g_libencloudLogLev = config.logLevel;

    return 0;
err:
    return ~0;
}

// Add prefix to path only if path is relative.
QString Config::_joinPaths (const QString &prefix, const QString &path)
{
    if (QDir(path).isAbsolute())
        return path;
    
    return QDir::cleanPath(prefix + QDir::separator() + path);
}

} // namespace libencloud

libencloud::Config *g_libencloudCfg = NULL;
int g_libencloudLogLev = LIBENCLOUD_LOG_MAX;
