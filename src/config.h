#ifndef _ECE_CONFIG_H_
#define _ECE_CONFIG_H_

#include <QDebug>
#include <QObject>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QTextStream>
#include "json.h"
#include "defaults.h"
#include "helpers.h"

using QtJson::JsonObject;
using QtJson::JsonArray;

namespace Ece {

typedef struct
{
    QUrl sbUrl;
    QFileInfo cacertPath;
    QFileInfo certPath;
    QFileInfo keyPath;
} 
ece_config_ssl_t;

typedef struct
{
    QUrl sbUrl;
    int timeout;
    QFileInfo prefix;

    ece_config_ssl_t sslInit;
    ece_config_ssl_t sslOp;

    int rsa_bits;
}
ece_config_t;

class Config
{
public:
    Config ();
    ~Config ();

    /* Configuration loading */
    int loadFromFile (const QString &filename);

    /* Configuration struct is publicly accessible */
    ece_config_t config;

private:
    int __parse (const JsonObject &jo);
    int __parse_sb (const JsonObject &jo);
    int __parse_sslInit (const JsonObject &jo);
    int __parse_sslOp (const JsonObject &jo);
    int __parse_ssl (const JsonObject &jo, ece_config_ssl_t &sc);
    QString __join_paths (const QString &s1, const QString &s2);
};

} // namespace Ece

#endif
