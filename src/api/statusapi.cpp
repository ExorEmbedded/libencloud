#include <encloud/Json>
#include <encloud/Api/StatusApi>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

StatusApi::StatusApi ()
    : Api ()
    , _state (StateNone)
    , _isFallback (-1)
{
    LIBENCLOUD_TRACE;

    connect(&_pollTimer, SIGNAL(timeout()), this, SLOT(_pollTimeout()));

    connect(&_client, SIGNAL(error(libencloud::Error, QVariant)),
            this, SLOT(_clientError(libencloud::Error)));
    connect(&_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>, QVariant)),
            this, SLOT(_clientComplete(QString)));
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

    jo = json::parse(response, ok).toMap();
    LIBENCLOUD_ERR_IF (!ok);

    LIBENCLOUD_ERR_IF (_parseState(jo["state"]));
    LIBENCLOUD_ERR_IF (_parseError(jo["error"]));
    LIBENCLOUD_ERR_IF (_parseProgress(jo["progress"]));
    LIBENCLOUD_ERR_IF (_parseFallback(jo["fallback"]));
    LIBENCLOUD_ERR_IF (_parseLogin(jo["login"]));
    LIBENCLOUD_ERR_IF (_parseNeed(jo["need"]));
err:
    return;
}

void StatusApi::_clientError (const libencloud::Error &err)
{
    switch (err.getCode())
    {
        // Server in our case is the Encloud Service
        case Error::CodeServerUnreach:
            emit error(Error(Error::CodeServiceUnreach));
            break;
        default:
            emit error(err);
            break;
    }
}

//
// private methods
//

int StatusApi::_parseState (const QVariant &v)
{
    State s = StateNone;

    if (!v.isNull())
       s = (State) v.toInt();

    if (s != _state)
        emit apiState((_state = s));

    return 0;
}

int StatusApi::_parseError (const QVariant &v)
{
    Error err;

    if (!v.isNull())
    {
        QVariantMap jo = v.toMap();

        if (!jo["code"].isNull())
            err.setCode((Error::Code) jo["code"].toInt());

        if (!jo["seq"].isNull())
            err.setSeq(jo["seq"].toInt());

        if (!jo["desc"].isNull())
            err.setDesc(jo["desc"].toString());

        if (!jo["extra"].isNull())
            err.setExtra(jo["extra"].toString());
    }

//    LIBENCLOUD_DBG("err: " << err.toString());

    // emit apiError, generic error and also state (for confirmation)
    if ((!err.isValid() && _error.isValid()) ||
            (err.isValid() && err != _error))
    {
        _error = err;

        emit apiError(_error);
        emit error(_error);
        emit apiState(_state);
    }

    return 0;
}

int StatusApi::_parseProgress (const QVariant &v)
{
    Progress p;
   
    if (!v.isNull())
    {
        QVariantMap jo = v.toMap();

        if (!jo["desc"].isNull())
            p.setDesc(jo["desc"].toString());

        if (!jo["step"].isNull())
            p.setStep(jo["step"].toInt());

        if (!jo["total"].isNull())
            p.setTotal(jo["total"].toInt());
    }

    if (p != _progress)
        emit apiProgress((_progress = p));

    return 0;
}

int StatusApi::_parseFallback (const QVariant &v)
{
    bool b = false;

    if (!v.isNull())
        b = v.toBool();

    if ((int)b != _isFallback)
        emit apiFallback((_isFallback = (int)b));

    return 0;
}

int StatusApi::_parseLogin (const QVariant &login)
{
    QVariantMap l = login.toMap();

    if (l != _login)
    {
        //LIBENCLOUD_DBG("login user: " << l["user"].toString());
        emit apiLogin(login);
        _login = l;
    }

    return 0;
}

int StatusApi::_parseNeed (const QVariant &v)
{
    QVariantMap needs;

    if (!v.isNull())
    {
        needs = v.toMap();

        //LIBENCLOUD_DBG("_needs: " << _needs << ", needs: " << needs);

        if (needs != _needs)
        {
            foreach (QString k, needs.keys())
                if (!_needs.contains(k))
                    emit apiNeed(k, needs[k]);

            _needs = needs;
        }
    } 
    else 
    {
        if (!_needs.isEmpty())
        {
            emit apiNeed("", QVariant());
            _needs.clear();
        }
    }

    return 0;
}

} // namespace libencloud
