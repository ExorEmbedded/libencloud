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

    this->config.sslInit.cacertPath = QFileInfo(ECE_CACERT_PATH);
    this->config.sslInit.certPath = QFileInfo(ECE_CERT1_PATH);
    this->config.sslInit.keyPath = QFileInfo(ECE_KEY1_PATH);

    this->config.sslOp.cacertPath = QFileInfo(ECE_CACERT_PATH);
    this->config.sslOp.certPath = QFileInfo(ECE_CERT2_PATH);
    this->config.sslOp.keyPath = QFileInfo(ECE_KEY2_PATH);

    this->config.rsaBits = ECE_RSA_BITS;
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
    config.timeout = jo["timeout"].toInt();
    ECE_DBG("timeout=" << config.timeout);

    config.csrTmplPath = __join_paths(config.prefix.absoluteFilePath(), \
            jo["csr"].toMap()["tmpl"].toString());
    ECE_DBG("csr tmpl=" << config.csrTmplPath.absoluteFilePath());

    ECE_ERR_IF (__parse_sb(jo["sb"].toMap()));
    ECE_ERR_IF (__parse_sslInit(jo["ssl_init"].toMap()));
    ECE_ERR_IF (__parse_sslOp(jo["ssl_op"].toMap()));

    config.rsaBits = jo["rsa"].toMap()["bits"].toInt();
    ECE_ERR_MSG_IF ((config.rsaBits == 0 || (config.rsaBits % 512) != 0),
            "rsa bits must be a multiple of 512!");
    ECE_DBG("rsa bits=" << config.rsaBits);

    return 0;
err:
    return ~0;
}

int Config::__parse_sb (const QVariantMap &jo)
{
    config.sbUrl = jo["url"].toString();

    ECE_DBG("sb url=" << config.sbUrl.toString());

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

int Config::__parse_ssl (const QVariantMap &jo, ece_config_ssl_t &sc)
{
    sc.cacertPath = __join_paths(config.prefix.absoluteFilePath(), jo["cacert"].toString());
    ECE_DBG("cacert=" << sc.cacertPath.absoluteFilePath());

    sc.certPath = __join_paths(config.prefix.absoluteFilePath(), jo["cert"].toString());
    ECE_DBG("cert=" << sc.certPath.absoluteFilePath());

    sc.keyPath = __join_paths(config.prefix.absoluteFilePath(), jo["key"].toString());
    ECE_DBG("key=" << sc.keyPath.absoluteFilePath());

    sc.sbUrl = jo["sb"].toMap()["url"].toString();
    if (sc.sbUrl.isEmpty())
        sc.sbUrl = config.sbUrl;
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
