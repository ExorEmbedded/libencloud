#include <QCoreApplication>
#include <QStringList>
#include "client.h"

namespace qurl {

Client::Client ()
    : m_error(false)
{
    connect(&m_client, SIGNAL(error(libencloud::Error)),
            this, SLOT(clientError(libencloud::Error)));
    connect(&m_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>)),
            this, SLOT(clientComplete(QString)));
}

bool Client::run (const QString &url, const QVariantMap &opts)
{
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration conf;
    QRegExp headerRx("(\\S+):\\ *(\\S+)");
    QRegExp dataRx("(\\S+)=(\\S+)");

    m_error = false;

    qDebug() << "url:" << url;
    qDebug() << "opts:" << opts;

    foreach (QString header, opts["headers"].toStringList())
    {
        if (header.indexOf(headerRx) == -1)
        {
            qDebug() << "Could not parse header:" << header;
            continue;
        }
        headers[headerRx.cap(1).toLower().toUtf8()] = headerRx.cap(2).toUtf8();
    }

    qDebug() << "headers:" << headers;

    QStringList postData = opts["postData"].toStringList();
    QString contentType;

    if (headers.contains("content-type"))
        contentType = headers["content-type"];

    // if no parameters are passed, we assume GET
    if (postData.size() == 0)
    {
        m_client.get(url, headers, conf);
    }
    // if json content types is specified, we assume plain data
    // (considering only first -d parameter)
    else if (contentType == "application/json")
    {
        m_client.post(url, headers, postData[0].toUtf8(), conf);
    }
    else  // otherwise assume application/x-www-form-urlencoded and setup query params
    {
        foreach (QString param, postData)
        {
            if (param.indexOf(dataRx) == -1)
            {
                qDebug() << "Not a valid keyval:" << param;
                continue;
            }

            qDebug() << "var:" << dataRx.cap(1);
            qDebug() << "val:" << dataRx.cap(2);
            params.addQueryItem(dataRx.cap(1), dataRx.cap(2));
        }

        m_client.run(url, params, headers, conf);
    }

    return true;
}

void Client::clientError (const libencloud::Error &err)
{
    qDebug() << "err: " << err.toString();

    m_error = true;

    qApp->quit();
}

void Client::clientComplete (const QString &response)
{
    fprintf(stdout, "%s", qPrintable(response));

    qApp->quit();
}

} // namespace Qurl
