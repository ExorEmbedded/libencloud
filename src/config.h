#ifndef _ENCLOUD_PRIV_CONFIG_H_
#define _ENCLOUD_PRIV_CONFIG_H_

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

namespace encloud {

typedef struct
{
    QUrl sbUrl;
    QFileInfo caPath;
    QFileInfo certPath;
    QFileInfo keyPath;
} 
encloud_config_ssl_t;

typedef struct
{
#ifndef ENCLOUD_TYPE_SECE
    QFileInfo serialPath;
    QFileInfo poiPath;
#endif
    
    QUrl sbUrl;
    int timeout;
    QFileInfo prefix;
    QFileInfo csrTmplPath;

    encloud_config_ssl_t sslInit;
    encloud_config_ssl_t sslOp;

    int rsaBits;

    int logLevel;
}
encloud_config_t;

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
    encloud_config_t config;
    QSettings *settings;

private:
    int __parse (const QVariantMap &jo);
    int __parse_sb (const QVariantMap &jo);
    int __parse_sslInit (const QVariantMap &jo);
    int __parse_sslOp (const QVariantMap &jo);
    int __parse_ssl (const QVariantMap &jo, encloud_config_ssl_t &sc);
    QString __join_paths (const QString &s1, const QString &s2);

    QVariant json;
};

} // namespace encloud

extern encloud::Config *g_cfg;

#endif  /* _ENCLOUD_PRIV_CONFIG_H_ */
