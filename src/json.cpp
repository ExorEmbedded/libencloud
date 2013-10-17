#include <QFile>
#include <QString>
#include <QVariant>
#include <QDebug>
#include "ece.h"
#include "helpers.h"
#include "config.h"
#include "json.h"

namespace EceJson {

QVariant parseFromFile (const QString &filename, bool &ok)
{
    QFile f(filename);
    QVariant json;

    ok = false;

    ECE_RETURN_MSG_IF (!f.open(QFile::ReadOnly | QFile::Text), json,
        "failed reading config file: " << filename);

    QTextStream ts(&f);
    QString js = ts.readAll();
    ECE_ERR_IF (js.isEmpty());

    json = EceJson::parse(js, ok);
    ECE_ERR_MSG_IF(!ok, "failed parsing Json!");

    ok = true;
err:
    return json;
}

} // namespace
