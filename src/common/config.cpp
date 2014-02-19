#include "config.h"
#include "helpers.h"

namespace libencloud {

/* Set defaults from defaults.h definitions */
Config::Config ()
    : settings(NULL)
{
    QString sep = "/";

#ifdef Q_OS_WIN32  // relative paths
    QString progFiles = QString(qgetenv("ProgramFiles"));
    QString appData = QString(qgetenv("AppData"));

    // main prefix and binaries are product-specific (e.g. 4iConnect)
    _prefix = progFiles + sep + QString(LIBENCLOUD_PRODUCTDIR);
    _sbinPrefix = _prefix + sep + LIBENCLOUD_SBIN_PREFIX;

    // configuration is package-specific (e.g. 4iConnect\libencloud) under %ProgramFiles%
    _confPrefix = progFiles + sep + QString(LIBENCLOUD_INSTALLDIR) +
            sep + LIBENCLOUD_ETC_PREFIX;

    // data is package-specific (4iConnect\libencloud) under %AppData%
    _dataPrefix = appData + sep + QString(LIBENCLOUD_INSTALLDIR) +
            sep + QString(LIBENCLOUD_DATA_PREFIX);

#else  // Linux - absolute paths
    _prefix = LIBENCLOUD_PREFIX_PATH;
    _confPrefix = LIBENCLOUD_ETC_PREFIX;
    _sbinPrefix = LIBENCLOUD_SBIN_PREFIX;
    _dataPrefix = LIBENCLOUD_DATA_PREFIX;
#endif

    filePath = QFileInfo(_confPrefix + sep + QString(LIBENCLOUD_CONF_FILE));

    // data are package-specific on win

    settings = new QSettings(LIBENCLOUD_ORG, LIBENCLOUD_APP);

#ifndef LIBENCLOUD_MODE_SECE
    config.serialPath = QFileInfo(_dataPrefix + LIBENCLOUD_SERIAL_FILE);
    config.poiPath = QFileInfo(_dataPrefix + LIBENCLOUD_POI_FILE);
#endif

    config.sbUrl = QUrl(LIBENCLOUD_SB_URL);
    config.timeout = LIBENCLOUD_RETRY_TIMEOUT;

    config.csrTmplPath = QFileInfo(_dataPrefix + LIBENCLOUD_CSRTMPL_FILE);

    config.sslInit.caPath = QFileInfo(_dataPrefix + LIBENCLOUD_INIT_CA_FILE);
    config.sslInit.certPath = QFileInfo(_dataPrefix + LIBENCLOUD_INIT_CERT_FILE);
    config.sslInit.keyPath = QFileInfo(_dataPrefix + LIBENCLOUD_INIT_KEY_FILE);

    config.sslOp.caPath = QFileInfo(_dataPrefix + LIBENCLOUD_INIT_CA_FILE);
    config.sslOp.certPath = QFileInfo(_dataPrefix + LIBENCLOUD_OP_CERT_FILE);
    config.sslOp.keyPath = QFileInfo(_dataPrefix + LIBENCLOUD_OP_KEY_FILE);

    config.rsaBits = LIBENCLOUD_RSA_BITS;

    config.vpnExePath = QFileInfo(_sbinPrefix + LIBENCLOUD_VPN_EXE_FILE);
    config.vpnConfPath = QFileInfo(_dataPrefix + LIBENCLOUD_VPN_CONF_FILE);
    config.vpnMgmtPort = LIBENCLOUD_VPN_MGMT_PORT;
    config.vpnVerbosity = LIBENCLOUD_VPN_VERBOSITY;

    config.logLevel = LIBENCLOUD_LOG_LEV;
}

Config::~Config()
{
    LIBENCLOUD_DELETE(settings);
}

QString Config::dump ()
{
    QString s;
    QTextStream ts(&s);
    bool ok;

    ts << "dumping configuration:";
    ts << endl;

    ts << "prefix=" << _prefix << endl;
    ts << "confPrefix=" << _confPrefix << endl;
    ts << "sbinPrefix=" << _sbinPrefix << endl;
    ts << "dataPrefix=" << _dataPrefix << endl;
    ts << "settings=" << settings->fileName() << endl;

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
        LIBENCLOUD_DBG("Failed parsing config file: " << filePath.absoluteFilePath());
        goto err;
    }

    LIBENCLOUD_ERR_IF (_parse(_json.toMap()));

    return 0;
err: 
    return ~0;
}

int Config::_parse (const QVariantMap &jo)
{
#ifndef LIBENCLOUD_MODE_SECE
    if (!jo["serial"].isNull())
        config.serialPath = _joinPaths(_dataPrefix, \
                jo["serial"].toString());

    if (!jo["poi"].isNull())
        config.poiPath = _joinPaths(_dataPrefix, \
                jo["poi"].toString());
#endif

    if (!jo["timeout"].isNull())
        config.timeout = jo["timeout"].toInt();

    if (!jo["csr"].toMap()["tmpl"].isNull())
        config.csrTmplPath = _joinPaths(_dataPrefix, \
                jo["csr"].toMap()["tmpl"].toString());

    if (!jo["sb"].isNull())
        LIBENCLOUD_ERR_IF (_parseSb(jo["sb"].toMap()));

    if (!jo["ssl_init"].isNull())
        LIBENCLOUD_ERR_IF (_parseSslInit(jo["ssl_init"].toMap()));

    if (!jo["ssl_op"].isNull())
        LIBENCLOUD_ERR_IF (_parseSslOp(jo["ssl_op"].toMap()));

    if (!jo["rsa"].toMap()["bits"].isNull())
    {
        config.rsaBits = jo["rsa"].toMap()["bits"].toInt();
        LIBENCLOUD_ERR_MSG_IF ((config.rsaBits == 0 || (config.rsaBits % 512) != 0),
                "rsa bits must be a multiple of 512!");
    }

    if (!jo["vpn"].isNull())
        LIBENCLOUD_ERR_IF (_parseVpn(jo["vpn"].toMap()));

    if (!jo["log"].toMap()["lev"].isNull())
    {
        config.logLevel = jo["log"].toMap()["lev"].toInt();
        LIBENCLOUD_ERR_MSG_IF ((config.logLevel < 0 || config.logLevel > 7),
                "log level must be between 0 and 7!");
    }

    return 0;
err:
    return ~0;
}

int Config::_parseSb (const QVariantMap &jo)
{
    if (!jo["url"].isNull())
        config.sbUrl = jo["url"].toString();

    return 0;
}

int Config::_parseSslInit (const QVariantMap &jo)
{
    return _parseSsl(jo, config.sslInit);
}

int Config::_parseSslOp (const QVariantMap &jo)
{
    return _parseSsl(jo, config.sslOp);
}

int Config::_parseSsl (const QVariantMap &jo, libencloud_config_ssl_t &sc)
{
    if (!jo["ca"].isNull())
        sc.caPath = _joinPaths(_dataPrefix, jo["ca"].toString());

    if (!jo["cert"].isNull())
        sc.certPath = _joinPaths(_dataPrefix, jo["cert"].toString());

    if (!jo["key"].isNull())
        sc.keyPath = _joinPaths(_dataPrefix, jo["key"].toString());

    sc.sbUrl = jo["sb"].toMap()["url"].toString();
    if (sc.sbUrl.isEmpty())
        sc.sbUrl = config.sbUrl;
    LIBENCLOUD_ERR_MSG_IF (sc.sbUrl.isEmpty(),
            "sb url undefined!");

    return 0;
err:
    return ~0;
}

int Config::_parseVpn (const QVariantMap &jo)
{
    if (!jo["path"].isNull())
        config.vpnExePath = _joinPaths(_sbinPrefix, jo["path"].toString());

    if (!jo["conf"].isNull())
        config.vpnConfPath = _joinPaths(_dataPrefix, jo["conf"].toString());

    if (!jo["mgmt"].toMap()["port"].isNull())
        config.vpnMgmtPort = jo["mgmt"].toMap()["port"].toInt();

    if (!jo["verb"].isNull())
        config.vpnVerbosity = jo["verb"].toInt();

    if (!jo["args"].isNull())
        config.vpnArgs = jo["args"].toString();

    return 0;
}

QString Config::_joinPaths (const QString &s1, const QString &s2)
{
    return QDir::cleanPath(s1 + QDir::separator() + s2);
}

} // namespace libencloud

libencloud::Config *g_cfg = NULL;
