#include "config.h"
#include "helpers.h"

namespace libencloud {

/* Set defaults from defaults.h definitions */
Config::Config ()
    : settings(NULL)
{
#ifdef Q_OS_WIN32
    _prefix = QString(qgetenv("ProgramFiles")) + QString("/") + QString(Q4IC_PRODUCT);
#else
    _prefix = LIBENCLOUD_PREFIX_PATH;
#endif
    _confPrefix = _prefix + LIBENCLOUD_ETC_PREFIX;
    filePath = QFileInfo(_confPrefix + QString("/") + QString(LIBENCLOUD_CONF_FILE));

    settings = new QSettings(LIBENCLOUD_ORG, LIBENCLOUD_APP);

#ifndef LIBENCLOUD_MODE_SECE
    config.serialPath = QFileInfo(_confPrefix + LIBENCLOUD_SERIAL_FILE);
    config.poiPath = QFileInfo(_confPrefix + LIBENCLOUD_POI_FILE);
#endif

    config.sbUrl = QUrl(LIBENCLOUD_SB_URL);
    config.timeout = LIBENCLOUD_TIMEOUT;

    config.csrTmplPath = QFileInfo(_confPrefix + LIBENCLOUD_CSRTMPL_FILE);

    config.sslInit.caPath = QFileInfo(_confPrefix + LIBENCLOUD_INIT_CA_FILE);
    config.sslInit.certPath = QFileInfo(_confPrefix + LIBENCLOUD_INIT_CERT_FILE);
    config.sslInit.keyPath = QFileInfo(_confPrefix + LIBENCLOUD_INIT_KEY_FILE);

    config.sslOp.caPath = QFileInfo(_confPrefix + LIBENCLOUD_INIT_CA_FILE);
    config.sslOp.certPath = QFileInfo(_confPrefix + LIBENCLOUD_OP_CERT_FILE);
    config.sslOp.keyPath = QFileInfo(_confPrefix + LIBENCLOUD_OP_KEY_FILE);

    config.rsaBits = LIBENCLOUD_RSA_BITS;

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

    LIBENCLOUD_ERR_IF (__parse(_json.toMap()));

    return 0;
err: 
    return ~0;
}

int Config::__parse (const QVariantMap &jo)
{
#ifndef LIBENCLOUD_MODE_SECE
    if (!jo["serial"].isNull())
        config.serialPath = __join_paths(_confPrefix, \
                jo["serial"].toString());

    if (!jo["poi"].isNull())
        config.poiPath = __join_paths(_confPrefix, \
                jo["poi"].toString());
#endif

    if (!jo["timeout"].isNull())
        config.timeout = jo["timeout"].toInt();

    if (!jo["csr"].toMap()["tmpl"].isNull())
        config.csrTmplPath = __join_paths(_confPrefix, \
                jo["csr"].toMap()["tmpl"].toString());

    if (!jo["sb"].isNull())
        LIBENCLOUD_ERR_IF (__parse_sb(jo["sb"].toMap()));

    if (!jo["ssl_init"].isNull())
        LIBENCLOUD_ERR_IF (__parse_sslInit(jo["ssl_init"].toMap()));

    if (!jo["ssl_op"].isNull())
        LIBENCLOUD_ERR_IF (__parse_sslOp(jo["ssl_op"].toMap()));

    if (!jo["rsa"].toMap()["bits"].isNull())
    {
        config.rsaBits = jo["rsa"].toMap()["bits"].toInt();
        LIBENCLOUD_ERR_MSG_IF ((config.rsaBits == 0 || (config.rsaBits % 512) != 0),
                "rsa bits must be a multiple of 512!");
    }

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

int Config::__parse_sb (const QVariantMap &jo)
{
    if (!jo["url"].isNull())
        config.sbUrl = jo["url"].toString();

    return 0;
}

int Config::__parse_sslInit (const QVariantMap &jo)
{
    return __parse_ssl(jo, config.sslInit);
}

int Config::__parse_sslOp (const QVariantMap &jo)
{
    return __parse_ssl(jo, config.sslOp);
}

int Config::__parse_ssl (const QVariantMap &jo, libencloud_config_ssl_t &sc)
{
    if (!jo["ca"].isNull())
        sc.caPath = __join_paths(_confPrefix, jo["ca"].toString());

    if (!jo["cert"].isNull())
        sc.certPath = __join_paths(_confPrefix, jo["cert"].toString());

    if (!jo["key"].isNull())
        sc.keyPath = __join_paths(_confPrefix, jo["key"].toString());

    sc.sbUrl = jo["sb"].toMap()["url"].toString();
    if (sc.sbUrl.isEmpty())
        sc.sbUrl = config.sbUrl;
    LIBENCLOUD_ERR_MSG_IF (sc.sbUrl.isEmpty(),
            "sb url undefined!");

    return 0;
err:
    return ~0;
}

QString Config::__join_paths (const QString &s1, const QString &s2)
{
    return QDir::cleanPath(s1 + QDir::separator() + s2);
}

} // namespace libencloud

libencloud::Config *g_cfg = NULL;
