#include <encloud/Json>
#include <encloud/Api/StatusApi>
#include <common/common.h>
#include <common/config.h>
#include <api/api.h>

namespace libencloud {

//
// public methods
//

StatusApi::StatusApi ()
    : Api ()
    , _state (StateNone)
{
    LIBENCLOUD_TRACE;

    connect(&_pollTimer, SIGNAL(timeout()), this, SLOT(_pollTimeout()));

    connect(&_client, SIGNAL(error(libencloud::Error)), this, SIGNAL(error(libencloud::Error)));
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

    // trigger a first request immediately
    _pollTimeout();

    // then start periodic poll timer
    _pollTimer.start(interval);
}

void StatusApi::stop ()
{
    LIBENCLOUD_TRACE;

    _pollTimer.stop();
}

//
// protected slots
//

void StatusApi::_pollTimeout ()
{
    QUrl url(getUrl());

    url.setPath(LIBENCLOUD_API_STATUS_PATH);

//    LIBENCLOUD_DBG("url: " << url.toString() << ", params: " << _params);

    _client.run(url, _params, _headers, _config);
}

void StatusApi::_clientComplete (const QString &response)
{
    bool ok;
    QVariantMap jo;

//    LIBENCLOUD_DBG("response: " << response);

    jo = json::parseJsonp(response, ok).toMap();
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

int StatusApi::_parseState (const QVariant &v)
{
    State s = (State) v.toInt();

    if (s != _state)
        emit apiState((_state = s));

    return 0;
}

int StatusApi::_parseError (const QVariant &v)
{
    QVariantMap jo = v.toMap();
    Error err;

    LIBENCLOUD_ERR_IF (jo["code"].isNull());

    if (!jo["code"].isNull())
        err.setCode((Error::Code) jo["code"].toInt());

    if (!jo["seq"].isNull())
        err.setSeq(jo["seq"].toInt());

    if (!jo["desc"].isNull())
        err.setDesc(jo["desc"].toString());

    if (!jo["extra"].isNull())
        err.setExtra(jo["extra"].toString());

    // emit apiError, generic error and also state (for confirmation)
    if (err != _error) {
        _error = err;
        emit apiError(_error);
        emit error(_error);
        emit apiState(_state);
    }

    return 0;
err:
    return ~0;
}

int StatusApi::_parseProgress (const QVariant &v)
{
    Progress p;
   
    QVariantMap vm = v.toMap();

    if (!vm["desc"].isNull())
        p.setDesc(vm["desc"].toString());

    if (!vm["step"].isNull())
        p.setStep(vm["step"].toInt());

    if (!vm["total"].isNull())
        p.setTotal(vm["total"].toInt());

    if (p != _progress)
        emit apiProgress((_progress = p));

    return 0;
}

int StatusApi::_parseNeed (const QVariant &v)
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
