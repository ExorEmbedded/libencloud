#include <QFile>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <encloud/Json>
#include "helpers.h"
#include "config.h"

namespace libencloud {
namespace json {

QVariant parseFromFile (const QString &filename, bool &ok)
{
    QFile f(filename);
    QVariant json;

    ok = false;

    LIBENCLOUD_RETURN_MSG_IF (!f.open(QFile::ReadOnly | QFile::Text), json,
        "failed reading config file: " << filename);

    QTextStream ts(&f);
    QString js = ts.readAll();
    LIBENCLOUD_ERR_IF (js.isEmpty());

    json = libencloud::json::parse(js, ok);
    LIBENCLOUD_ERR_MSG_IF(!ok, "failed parsing Json!");

    ok = true;
err:
    return json;
}

QVariant parseJsonp (const QString &str, bool &ok)
{
    QRegExp jsonpRx("\\w+\\((.*)\\)");
    QString json;

    jsonpRx.indexIn(str);

    LIBENCLOUD_ERR_IF (jsonpRx.captureCount() != 1);

    json = jsonpRx.cap(1);

    return libencloud::json::parse(json, ok);

err:
    ok = false;
    return QVariant();
}

} // namespace json
} // namespace libencloud
