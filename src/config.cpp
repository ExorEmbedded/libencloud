#include "config.h"
#include "helpers.h"

namespace Ece {

/* Set defaults from defaults.h definitions */
Config::Config()
{
    ECE_TRACE;

    this->config.sbUrl = QUrl(ECE_SB_URL);
    this->config.timeout = ECE_TIMEOUT;
    this->config.prefix = QFileInfo(ECE_PREFIX_PATH);

    this->config.sslInit.cacertPath = QFileInfo(ECE_CACERT_PATH);
    this->config.sslInit.certPath = QFileInfo(ECE_CERT1_PATH);
    this->config.sslInit.keyPath = QFileInfo(ECE_KEY1_PATH);

    this->config.sslOp.cacertPath = QFileInfo(ECE_CACERT_PATH);
    this->config.sslOp.certPath = QFileInfo(ECE_CERT2_PATH);
    this->config.sslOp.keyPath = QFileInfo(ECE_KEY2_PATH);
}

Config::~Config()
{
    ECE_TRACE;
}

/* Read config from file */
int Config::loadFromFile(const QString &filename)
{
    ECE_TRACE;

    QString fn = __join_paths(QString(ECE_PREFIX_PATH), filename);
    ECE_DBG("filename: " << fn);

    QFile f(fn);
    ECE_RETURN_MSG_IF (!f.open(QFile::ReadOnly | QFile::Text), ~0, 
        "failed reading config file: " << fn);

    QTextStream ts(&f);
    QString json = ts.readAll();
    ECE_RETURN_IF (json.isEmpty(), ~0);
    
    bool ok;
    JsonObject jo = QtJson::parse(json, ok).toMap();
    ECE_ERR_IF (!ok);

    ECE_DBG(QtJson::serialize(jo));
    ECE_ERR_IF (__parse(jo));

    return 0;
err: 
    return ~0;
}

int Config::__parse(const JsonObject &jo)
{
    config.timeout = jo["timeout"].toInt();
    config.prefix = jo["prefix"].toString();

    ECE_DBG("timeout: " << config.timeout);

    ECE_ERR_IF (__parse_sb(jo["sb"].toMap()));
    ECE_ERR_IF (__parse_sslInit(jo["ssl_init"].toMap()));
    ECE_ERR_IF (__parse_sslOp(jo["ssl_op"].toMap()));

    return 0;
err:
    return ~0;
}

int Config::__parse_sb(const JsonObject &jo)
{
    config.sbUrl = jo["url"].toString();

    ECE_DBG("sb url: " << config.sbUrl.toString());

    return 0;
}

int Config::__parse_sslInit(const JsonObject &jo)
{
    return __parse_ssl(jo, config.sslInit);
}

int Config::__parse_sslOp(const JsonObject &jo)
{
    return __parse_ssl(jo, config.sslOp);
}

int Config::__parse_ssl(const JsonObject &jo, ece_config_ssl_t &sc)
{
    sc.cacertPath = __join_paths(config.prefix.absoluteFilePath(), jo["cacert"].toString());
    ECE_DBG("cacert: " << sc.cacertPath.absoluteFilePath());

    sc.certPath = __join_paths(config.prefix.absoluteFilePath(), jo["cert"].toString());
    ECE_DBG("cert: " << sc.certPath.absoluteFilePath());

    sc.keyPath = __join_paths(config.prefix.absoluteFilePath(), jo["key"].toString());
    ECE_DBG("key: " << sc.keyPath.absoluteFilePath());

    sc.sbUrl = jo["sb"].toMap()["url"].toString();
    if (sc.sbUrl.isEmpty())
        sc.sbUrl = config.sbUrl;
    ECE_DBG("sb url: " << sc.sbUrl.toString());

    return 0;
}

QString Config::__join_paths(const QString &s1, const QString &s2)
{
    return QDir::cleanPath(s1 + QDir::separator() + s2);
}

} // namespace Ece
