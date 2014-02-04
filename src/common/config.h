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
#ifndef LIBENCLOUD_TYPE_SECE
    QFileInfo serialPath;
    QFileInfo poiPath;
#endif
    
    QUrl sbUrl;
    int timeout;
    QFileInfo prefix;
    QFileInfo csrTmplPath;

    libencloud_config_ssl_t sslInit;
    libencloud_config_ssl_t sslOp;

    int rsaBits;

    int logLevel;
}
libencloud_config_t;

class Config
{
public:
    Config ();
    ~Config ();

    /* Configuration loading */
    int loadFromFile (QString filename);
    static QVariantMap fileToJson (QString filename);
    QString dump ();

    /* Configuration objects are publicly accessible */
    libencloud_config_t config;
    QSettings *settings;

private:
    int __parse (const QVariantMap &jo);
    int __parse_sb (const QVariantMap &jo);
    int __parse_sslInit (const QVariantMap &jo);
    int __parse_sslOp (const QVariantMap &jo);
    int __parse_ssl (const QVariantMap &jo, libencloud_config_ssl_t &sc);
    QString __join_paths (const QString &s1, const QString &s2);

    QVariant json;
};

} // namespace libencloud

extern libencloud::Config *g_cfg;

#endif  /* _LIBENCLOUD_PRIV_CONFIG_H_ */
