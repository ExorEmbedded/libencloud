#include "config.h"
#include "helpers.h"
// don't depend on Qt for debug so we can print errors before log level is set
#undef __ECE_MSG
#define __ECE_MSG(lev, levstr, msg) __ECE_PRINT(lev, levstr, msg)

namespace Ece {

/* Set defaults from defaults.h definitions */
Config::Config ()
    : settings(NULL)
{
    this->settings = new QSettings(ECE_SETTINGS_ORG, ECE_SETTINGS_APP);

#ifndef ECE_TYPE_SECE
    this->config.serialPath = QFileInfo(ECE_SERIAL_PATH);
    this->config.poiPath = QFileInfo(ECE_POI_PATH);
#endif

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

    ts << "prefix= " << ECE_PREFIX_PATH << endl;
    ts << "settings=%s" << this->settings->fileName() << endl;

    ts << EceJson::serialize(this->json, ok);

    return s;
}

/* Read config from file */
int Config::loadFromFile (QString filename)
{
    bool ok;

    QString fn = __join_paths(QString(ECE_PREFIX_PATH), filename);

    this->json = EceJson::parseFromFile(fn, ok);
    ECE_ERR_IF (!ok);

    ECE_ERR_IF (__parse(this->json.toMap()));

    return 0;
err: 
    return ~0;
}

int Config::__parse (const QVariantMap &jo)
{
#ifndef ECE_TYPE_SECE
    if (!jo["serial"].isNull())
        this->config.serialPath = __join_paths(this->config.prefix.absoluteFilePath(), \
                jo["serial"].toString());

    if (!jo["poi"].isNull())
        this->config.poiPath = __join_paths(this->config.prefix.absoluteFilePath(), \
                jo["poi"].toString());
#endif

    if (!jo["timeout"].isNull())
        this->config.timeout = jo["timeout"].toInt();

    if (!jo["csr"].toMap()["tmpl"].isNull())
        this->config.csrTmplPath = __join_paths(this->config.prefix.absoluteFilePath(), \
                jo["csr"].toMap()["tmpl"].toString());

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

    if (!jo["log"].toMap()["lev"].isNull())
    {
        this->config.logLevel = jo["log"].toMap()["lev"].toInt();
        ECE_ERR_MSG_IF ((this->config.logLevel < 0 || this->config.logLevel > 7),
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

int Config::__parse_ssl (const QVariantMap &jo, ece_config_ssl_t &sc)
{
    if (!jo["ca"].isNull())
        sc.caPath = __join_paths(this->config.prefix.absoluteFilePath(), jo["ca"].toString());

    if (!jo["cert"].isNull())
        sc.certPath = __join_paths(this->config.prefix.absoluteFilePath(), jo["cert"].toString());

    if (!jo["key"].isNull())
        sc.keyPath = __join_paths(this->config.prefix.absoluteFilePath(), jo["key"].toString());

    sc.sbUrl = jo["sb"].toMap()["url"].toString();
    if (sc.sbUrl.isEmpty())
        sc.sbUrl = this->config.sbUrl;
    ECE_ERR_MSG_IF (sc.sbUrl.isEmpty(),
            "sb url undefined!");

    return 0;
err:
    return ~0;
}

QString Config::__join_paths (const QString &s1, const QString &s2)
{
    return QDir::cleanPath(s1 + QDir::separator() + s2);
}

} // namespace Ece
