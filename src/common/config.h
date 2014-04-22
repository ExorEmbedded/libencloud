#ifndef _LIBENCLOUD_PRIV_CONFIG_H_
#define _LIBENCLOUD_PRIV_CONFIG_H_

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QUrl>
#include <encloud/Common>
#include <encloud/Json>
#include "crypto.h"
#include "defaults.h"
#include "helpers.h"

namespace libencloud {

typedef struct
{
    QUrl sbUrl;
    QString auth;
    QFileInfo caPath;
    QFileInfo certPath;
    QFileInfo keyPath;
} 
libencloud_config_ssl_t;

typedef struct
{
    QString bind;

#ifdef LIBENCLOUD_MODE_ECE
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
    QFileInfo fallbackVpnConfPath;
    int vpnMgmtPort;
    int vpnVerbosity;
    QString vpnArgs;

    int logLevel;
    QString logTo;
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
    QSettings *sysSettings;
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
    int _parseLog (const QVariantMap &jo);
    QString _joinPaths (const QString &s1, const QString &s2);

    QVariant _json;

    QString _prefix;
    QString _confPrefix;
    QString _sbinPrefix;
    QString _dataPrefix;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CONFIG_H_ */
