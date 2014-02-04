#include "config.h"
#include "helpers.h"

namespace libencloud {

/* Set defaults from defaults.h definitions */
Config::Config ()
    : settings(NULL)
{
#ifdef Q_OS_WIN32
    this->prefix = QString(qgetenv("ProgramFiles")) + QString("/") + QString(Q4IC_PRODUCT);
#else
    this->prefix = LIBENCLOUD_PREFIX_PATH;
#endif
    this->confPrefix = this->prefix + LIBENCLOUD_ETC_PREFIX;
    this->filePath = QFileInfo(this->confPrefix + QString("/") + QString(LIBENCLOUD_CONF_FILE));

    this->settings = new QSettings(LIBENCLOUD_ORG, LIBENCLOUD_APP);

#ifndef LIBENCLOUD_TYPE_SECE
    this->config.serialPath = QFileInfo(this->confPrefix + LIBENCLOUD_SERIAL_FILE);
    this->config.poiPath = QFileInfo(this->confPrefix + LIBENCLOUD_POI_FILE);
#endif

    this->config.sbUrl = QUrl(LIBENCLOUD_SB_URL);
    this->config.timeout = LIBENCLOUD_TIMEOUT;

    this->config.csrTmplPath = QFileInfo(this->confPrefix + LIBENCLOUD_CSRTMPL_FILE);

    this->config.sslInit.caPath = QFileInfo(this->confPrefix + LIBENCLOUD_INIT_CA_FILE);
    this->config.sslInit.certPath = QFileInfo(this->confPrefix + LIBENCLOUD_INIT_CERT_FILE);
    this->config.sslInit.keyPath = QFileInfo(this->confPrefix + LIBENCLOUD_INIT_KEY_FILE);

    this->config.sslOp.caPath = QFileInfo(this->confPrefix + LIBENCLOUD_INIT_CA_FILE);
    this->config.sslOp.certPath = QFileInfo(this->confPrefix + LIBENCLOUD_OP_CERT_FILE);
    this->config.sslOp.keyPath = QFileInfo(this->confPrefix + LIBENCLOUD_OP_KEY_FILE);

    this->config.rsaBits = LIBENCLOUD_RSA_BITS;

    this->config.logLevel = LIBENCLOUD_LOG_LEV;
}

Config::~Config()
{
    if (this->settings)
        delete this->settings;
}

QString Config::dump ()
{
    QString s;
    QTextStream ts(&s);
    bool ok;

    ts << "dumping configuration:";
    ts << endl;

    ts << "prefix=" << this->prefix << endl;
    ts << "settings=" << this->settings->fileName() << endl;

    ts << libencloud::json::serialize(this->json, ok);

    return s;
}

/* Read config from file */
int Config::loadFromFile ()
{
    bool ok;

    this->json = libencloud::json::parseFromFile(this->filePath.absoluteFilePath(), ok);
    if (!ok || this->json.isNull())
    {
        LIBENCLOUD_DBG("Failed parsing config file: " << this->filePath.absoluteFilePath());
        goto err;
    }

    LIBENCLOUD_ERR_IF (__parse(this->json.toMap()));

    return 0;
err: 
    return ~0;
}

int Config::__parse (const QVariantMap &jo)
{
#ifndef LIBENCLOUD_TYPE_SECE
    if (!jo["serial"].isNull())
        this->config.serialPath = __join_paths(this->confPrefix, \
                jo["serial"].toString());

    if (!jo["poi"].isNull())
        this->config.poiPath = __join_paths(this->confPrefix, \
                jo["poi"].toString());
#endif

    if (!jo["timeout"].isNull())
        this->config.timeout = jo["timeout"].toInt();

    if (!jo["csr"].toMap()["tmpl"].isNull())
        this->config.csrTmplPath = __join_paths(this->confPrefix, \
                jo["csr"].toMap()["tmpl"].toString());

    if (!jo["sb"].isNull())
        LIBENCLOUD_ERR_IF (__parse_sb(jo["sb"].toMap()));

    if (!jo["ssl_init"].isNull())
        LIBENCLOUD_ERR_IF (__parse_sslInit(jo["ssl_init"].toMap()));

    if (!jo["ssl_op"].isNull())
        LIBENCLOUD_ERR_IF (__parse_sslOp(jo["ssl_op"].toMap()));

    if (!jo["rsa"].toMap()["bits"].isNull())
    {
        this->config.rsaBits = jo["rsa"].toMap()["bits"].toInt();
        LIBENCLOUD_ERR_MSG_IF ((this->config.rsaBits == 0 || (this->config.rsaBits % 512) != 0),
                "rsa bits must be a multiple of 512!");
    }

    if (!jo["log"].toMap()["lev"].isNull())
    {
        this->config.logLevel = jo["log"].toMap()["lev"].toInt();
        LIBENCLOUD_ERR_MSG_IF ((this->config.logLevel < 0 || this->config.logLevel > 7),
                "log level must be between 0 and 7!");
    }

    return 0;
err:
    return ~0;
}

int Config::__parse_sb (const QVariantMap &jo)
{
    if (!jo["url"].isNull())
        this->config.sbUrl = jo["url"].toString();

    return 0;
}

int Config::__parse_sslInit (const QVariantMap &jo)
{
    return __parse_ssl(jo, this->config.sslInit);
}

int Config::__parse_sslOp (const QVariantMap &jo)
{
    return __parse_ssl(jo, this->config.sslOp);
}

int Config::__parse_ssl (const QVariantMap &jo, libencloud_config_ssl_t &sc)
{
    if (!jo["ca"].isNull())
        sc.caPath = __join_paths(this->confPrefix, jo["ca"].toString());

    if (!jo["cert"].isNull())
        sc.certPath = __join_paths(this->confPrefix, jo["cert"].toString());

    if (!jo["key"].isNull())
        sc.keyPath = __join_paths(this->confPrefix, jo["key"].toString());

    sc.sbUrl = jo["sb"].toMap()["url"].toString();
    if (sc.sbUrl.isEmpty())
        sc.sbUrl = this->config.sbUrl;
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
