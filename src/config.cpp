#include "config.h"
#include "helpers.h"

namespace Ece {

/* Set defaults from defaults.h definitions */
Config::Config ()
    : settings(NULL)
{
    ECE_TRACE;

    ECE_DBG("prefix=" << ECE_PREFIX_PATH);

    this->settings = new QSettings(ECE_SETTINGS_ORG, ECE_SETTINGS_APP);

    this->config.sbUrl = QUrl(ECE_SB_URL);
    this->config.timeout = ECE_TIMEOUT;
    this->config.prefix = QFileInfo(ECE_PREFIX_PATH);

    this->config.csrTmplPath = QFileInfo(ECE_CSRTMPL_PATH);

    this->config.sslInit.caPath = QFileInfo(ECE_INIT_CA_PATH);
    this->config.sslInit.certPath = QFileInfo(ECE_INIT_CERT_PATH);
    this->config.sslInit.keyPath = QFileInfo(ECE_INIT_KEY_PATH);

    this->config.sslOp.caPath = QFileInfo(ECE_INIT_CA_PATH);
    this->config.sslOp.certPath = QFileInfo(ECE_OP_CERT_PATH);
    this->config.sslOp.keyPath = QFileInfo(ECE_OP_KEY_PATH);

    this->config.rsaBits = ECE_RSA_BITS;

    this->config.logLevel = ECE_LOG_LEV;
}

Config::~Config()
{
    ECE_TRACE;

    if (this->settings)
        delete this->settings;
}

/* Read config from file */
int Config::loadFromFile (QString filename)
{
    ECE_TRACE;
    bool ok;

    QString fn = __join_paths(QString(ECE_PREFIX_PATH), filename);
    ECE_DBG("filename=" << fn);

    QVariant json = EceJson::parseFromFile(fn, ok);
    ECE_ERR_IF (!ok);

    ECE_DBG(EceJson::serialize(json, ok));
    ECE_ERR_IF (!ok);

    ECE_ERR_IF (__parse(json.toMap()));

    return 0;
err: 
    return ~0;
}

int Config::__parse (const QVariantMap &jo)
{
    if (!jo["timeout"].isNull())
        this->config.timeout = jo["timeout"].toInt();
    ECE_DBG("timeout=" << this->config.timeout);

    if (!jo["csr"].toMap()["tmpl"].isNull())
        this->config.csrTmplPath = __join_paths(this->config.prefix.absoluteFilePath(), \
                jo["csr"].toMap()["tmpl"].toString());
    ECE_DBG("csr tmpl=" << this->config.csrTmplPath.absoluteFilePath());

    if (!jo["sb"].isNull())
        ECE_ERR_IF (__parse_sb(jo["sb"].toMap()));

    if (!jo["ssl_init"].isNull())
        ECE_ERR_IF (__parse_sslInit(jo["ssl_init"].toMap()));

    if (!jo["ssl_op"].isNull())
        ECE_ERR_IF (__parse_sslOp(jo["ssl_op"].toMap()));

    if (!jo["rsa"].toMap()["bits"].isNull())
    {
        this->config.rsaBits = jo["rsa"].toMap()["bits"].toInt();
        ECE_ERR_MSG_IF ((this->config.rsaBits == 0 || (this->config.rsaBits % 512) != 0),
                "rsa bits must be a multiple of 512!");
    }
    ECE_DBG("rsa bits=" << this->config.rsaBits);

    if (!jo["log"].toMap()["lev"].isNull())
    {
        this->config.logLevel = jo["log"].toMap()["lev"].toInt();
        ECE_ERR_MSG_IF ((this->config.logLevel < 0 || this->config.logLevel > 7),
                "log level must be between 0 and 7!");
    }
    ECE_DBG("log lev=" << this->config.logLevel);

    return 0;
err:
    return ~0;
}

int Config::__parse_sb (const QVariantMap &jo)
{
    if (!jo["url"].isNull())
        this->config.sbUrl = jo["url"].toString();

    ECE_DBG("sb url=" << this->config.sbUrl.toString());

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

int Config::__parse_ssl (const QVariantMap &jo, ece_config_ssl_t &sc)
{
    if (!jo["ca"].isNull())
        sc.caPath = __join_paths(this->config.prefix.absoluteFilePath(), jo["ca"].toString());
    ECE_DBG("ca=" << sc.caPath.absoluteFilePath());

    if (!jo["cert"].isNull())
        sc.certPath = __join_paths(this->config.prefix.absoluteFilePath(), jo["cert"].toString());
    ECE_DBG("cert=" << sc.certPath.absoluteFilePath());

    if (!jo["key"].isNull())
        sc.keyPath = __join_paths(this->config.prefix.absoluteFilePath(), jo["key"].toString());
    ECE_DBG("key=" << sc.keyPath.absoluteFilePath());

    sc.sbUrl = jo["sb"].toMap()["url"].toString();
    if (sc.sbUrl.isEmpty())
        sc.sbUrl = this->config.sbUrl;
    ECE_ERR_MSG_IF (sc.sbUrl.isEmpty(),
            "sb url undefined!");
    ECE_DBG("sb url=" << sc.sbUrl.toString());

    return 0;
err:
    return ~0;
}

QString Config::__join_paths (const QString &s1, const QString &s2)
{
    return QDir::cleanPath(s1 + QDir::separator() + s2);
}

} // namespace Ece
