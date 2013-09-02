#ifndef _ECE_JSON_H_
#define _ECE_JSON_H_

#include <QVariant>
#include <QString>

namespace EceJson {

    // implementation-specific interfaces (defined in json-xxx.cpp)
    QVariant parse (const QString &str, bool &ok);
    QString serialize (const QVariant &json, bool &ok);

    // generic helpers (defined in json.cpp)
    QVariant parseFromFile (const QString &filename, bool &ok);
}

#endif
