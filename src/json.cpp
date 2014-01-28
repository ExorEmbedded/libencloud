#include <QFile>
#include <QString>
#include <QVariant>
#include <QDebug>
#include "helpers.h"
#include "config.h"
#include "json.h"

namespace encloud {
namespace json {

QVariant parseFromFile (const QString &filename, bool &ok)
{
    QFile f(filename);
    QVariant json;

    ok = false;

    ENCLOUD_RETURN_MSG_IF (!f.open(QFile::ReadOnly | QFile::Text), json,
        "failed reading config file: " << filename);

    QTextStream ts(&f);
    QString js = ts.readAll();
    ENCLOUD_ERR_IF (js.isEmpty());

    json = encloud::json::parse(js, ok);
    ENCLOUD_ERR_MSG_IF(!ok, "failed parsing Json!");

    ok = true;
err:
    return json;
}

} // namespace json
} // namespace encloud
