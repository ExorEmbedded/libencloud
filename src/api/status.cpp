#include <QRegExp>
#include <common/common.h>
#include <common/config.h>
#include <encloud/Json>
#include <encloud/Api/Status>

#define LIBENCLOUD_API_STATUS_PATH  "/api_v1/status"

namespace libencloud {

//
// public methods
//

StatusApi::StatusApi ()
    : _state (StateNone)
{
    LIBENCLOUD_TRACE;

    connect(&_timer, SIGNAL(timeout()), this, SLOT(_timeout()));

    _client.setDebug(false);
    connect(&_client, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
    connect(&_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));
}

StatusApi::~StatusApi ()
{
    LIBENCLOUD_TRACE;

    stop();
}

void StatusApi::start (int interval)
{
    LIBENCLOUD_TRACE;

    _timer.start(interval);
}

void StatusApi::stop ()
{
    LIBENCLOUD_TRACE;

    _timer.stop();
}

//
// private slots
//

void StatusApi::_timeout ()
{
    //LIBENCLOUD_TRACE;

    // TODO from config/registry
    QUrl url("http://localhost:4884");
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration config;

    url.setPath(LIBENCLOUD_API_STATUS_PATH);

    _client.run(url, params, headers, config);
}

void StatusApi::_clientComplete (const QString &response)
{
    bool ok;
    QRegExp jsonpRx("\\w+\\((.*)\\)");
    QString json;
    QVariantMap jo;

    jsonpRx.indexIn(response);

    LIBENCLOUD_ERR_IF (jsonpRx.captureCount() != 1);
    json = jsonpRx.cap(1);

//    LIBENCLOUD_DBG("response: " << json);

    jo = json::parse(json, ok).toMap();
    LIBENCLOUD_ERR_IF (!ok);

    if (!jo["state"].isNull())
        LIBENCLOUD_ERR_IF (_parseState(jo["state"]));

    if (!jo["error"].isNull())
        LIBENCLOUD_ERR_IF (_parseError(jo["error"]));

    if (!jo["progress"].isNull())
        LIBENCLOUD_ERR_IF (_parseProgress(jo["progress"]));

    if (!jo["need"].isNull())
        LIBENCLOUD_ERR_IF (_parseNeed(jo["need"]));

err:
    return;
}

//
// private methods
//

int StatusApi::_parseState (QVariant v)
{
    State s = (State) v.toInt();

    if (s != _state)
        emit apiState((_state = s));

    return 0;
}

int StatusApi::_parseError (QVariant v)
{
    QString err = v.toString();

    if (err != _error)
        emit apiError((_error = err));

    return 0;
}

int StatusApi::_parseProgress (QVariant v)
{
    Progress p;
   
    QVariantMap vm = v.toMap();

    if (!vm["desc"].isNull())
        p.setDesc(vm["desc"].toString());

    if (!vm["step"].isNull())
        p.setStep(vm["step"].toInt());

    if (!vm["total"].isNull())
        p.setTotal(vm["total"].toInt());

    if (!(p == _progress))
        emit apiProgress((_progress = p));

    return 0;
}

int StatusApi::_parseNeed (QVariant v)
{
    QStringList needs = v.toString().split(" ");

    if (needs != _needs)
    {
        foreach (QString n, needs)
            if (!_needs.contains(n))
                emit apiNeed(n);

        _needs = needs;
    }

    return 0;
}

} // namespace libencloud