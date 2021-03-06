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
#ifndef Q_OS_WINCE
#include <encloud/Crypto>
#endif
#include <encloud/Json>
#include "defaults.h"
#include "helpers.h"

namespace libencloud {

typedef struct
{
    QUrl sbUrl;
    QString auth;
    QString authFormat;
    bool verifyCA;
    QFileInfo caPath;
    QFileInfo certPath;
    QFileInfo keyPath;
    QFileInfo p12Path;
} 
libencloud_config_ssl_t;

typedef struct
{
    bool isProfile;     // is this a Connection Profile?
                        // true: paths relative to OpenVPN config path are used
                        // false: we use absolute paths constructed from data prefix
    QString bind;

    
    QUrl regUrl, sbUrl;
    int timeout;
    bool autoretry;
    bool decongest;
    bool compat;

    libencloud_config_ssl_t ssl;
    libencloud_config_ssl_t sslInit;
    libencloud_config_ssl_t sslOp;

    int rsaBits;

    bool setupEnabled;
    bool setupAgent;
    QFileInfo regProvisioningPath;

    QFileInfo vpnExePath;
    QFileInfo vpnConfPath;
    QFileInfo fallbackVpnConfPath;
    int vpnMgmtPort;
    int vpnMgmtPeriod;
    int vpnVerbosity;
    QString vpnArgs;
    bool vpnFw;

    int logLevel;
    QString logTo;
}
libencloud_config_t;

class Config : public QObject
{
    Q_OBJECT

public:
    Config ();
    ~Config ();

    /* Configuration loading and debugging */
    int loadFromFile ();
    QVariantMap getMap ();
    QString dump ();

    /* Configuration objects are publicly accessible */
    libencloud_config_t config;
    QSettings *sysSettings;
#ifndef Q_OS_WINCE
    libencloud_crypto_t crypto;
#endif

    /* Path to configuration file */
    QFileInfo filePath;

    QString confPrefix;
    QString sbinPrefix;
    QString dataPrefix;
    QString logPrefix;
    QString userDataPrefix;

protected slots:

    void receive (const QVariant &cfg); 

protected:
    int _loadExt ();
    int _parse (const QVariantMap &jo);
    int _parsePaths (const QVariantMap &jo);
    int _parseRegistry (const QVariantMap &jo);
    int _parseSb (const QVariantMap &jo);
    int _parseSslInit (const QVariantMap &jo);
    int _parseSslOp (const QVariantMap &jo);
    int _parseSsl (const QVariantMap &jo, libencloud_config_ssl_t &sc);
    int _parseVpn (const QVariantMap &jo);
    int _parseLog (const QVariantMap &jo);
    QString _joinPaths (const QString &prefix, const QString &path, bool keepRelative = false);

    QVariantMap _json;
    QVariantMap _jsonOrig;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CONFIG_H_ */
