#ifndef _LIBENCLOUD_PRIV_CONFIG_H_
#define _LIBENCLOUD_PRIV_CONFIG_H_

#include <QDebug>
#include <QObject>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QTextStream>
#include <QSettings>
#include "json.h"
#include "defaults.h"
#include "helpers.h"
#include "crypto.h"

namespace libencloud {

typedef struct
{
    QUrl sbUrl;
    QFileInfo caPath;
    QFileInfo certPath;
    QFileInfo keyPath;
} 
libencloud_config_ssl_t;

typedef struct
{
#ifndef LIBENCLOUD_MODE_SECE
    QFileInfo serialPath;
    QFileInfo poiPath;
#endif
    
    QUrl sbUrl;
    int timeout;
    QFileInfo csrTmplPath;

    libencloud_config_ssl_t sslInit;
    libencloud_config_ssl_t sslOp;

    int rsaBits;

    QFileInfo vpnExePath;
    QFileInfo vpnConfPath;
    int vpnMgmtPort;
    int vpnVerbosity;
    QString vpnArgs;

    int logLevel;
}
libencloud_config_t;

class Config
{
public:
    Config ();
    ~Config ();

    /* Configuration loading */
    int loadFromFile ();
    static QVariantMap fileToJson (QString filename);
    QString dump ();

    /* Configuration objects are publicly accessible */
    libencloud_config_t config;
    QSettings *settings;
    libencloud_crypto_t crypto;

    /* Path to configuration file */
    QFileInfo filePath;

private:
    int _parse (const QVariantMap &jo);
    int _parseSb (const QVariantMap &jo);
    int _parseSslInit (const QVariantMap &jo);
    int _parseSslOp (const QVariantMap &jo);
    int _parseSsl (const QVariantMap &jo, libencloud_config_ssl_t &sc);
    int _parseVpn (const QVariantMap &jo);
    QString _joinPaths (const QString &s1, const QString &s2);

    QVariant _json;

    QString _prefix;
    QString _confPrefix;
    QString _sbinPrefix;
    QString _dataPrefix;
};

} // namespace libencloud

extern libencloud::Config *g_cfg;

#endif  /* _LIBENCLOUD_PRIV_CONFIG_H_ */
