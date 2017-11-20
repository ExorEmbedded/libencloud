#include <encloud/Logger>
#include <encloud/Watchdog>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

Watchdog::Watchdog ()
    : _url(LIBENCLOUD_WATCHDOG_URL)
    , _interval(LIBENCLOUD_WATCHDOG_TOUT)
    , _running(false)
    , _state(UnknownState)
{
    LIBENCLOUD_TRACE;

    connect(&_timer, SIGNAL(timeout()), this, SLOT(_timeout()));

    _client.setDebug(false);
    connect(&_client, SIGNAL(error(libencloud::Error, QVariant)),
            this, SLOT(_clientError(libencloud::Error)));
    connect(&_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>, QVariant)),
            this, SLOT(_clientComplete(QString)));
}

Watchdog::~Watchdog ()
{
    LIBENCLOUD_TRACE;

    stop();
}

int Watchdog::setUrl (const QUrl &url)
{
    LIBENCLOUD_RETURN_IF (!url.isValid(), ~0);

    _url = url;
    return 0;
}

int Watchdog::setInterval (int msecs)
{
    LIBENCLOUD_RETURN_IF (msecs <= 0, ~0);

    _interval = msecs;
    return 0;
}

int Watchdog::start ()
{
    LIBENCLOUD_TRACE;

    _timer.start(_interval);

    _running = true;

    return 0;
}

int Watchdog::stop ()
{
    LIBENCLOUD_TRACE;

    _timer.stop();

    _running = false;

    return 0;
}

//
// protected slots
// 

void Watchdog::_timeout ()
{
    LIBENCLOUD_TRACE;

    // logging tests
#if 0
    LIBENCLOUD_DBG("[Watchdog] debug!!!!");
    LIBENCLOUD_ERR("[Watchdog] ERR");
    LIBENCLOUD_RETURN_IF(1 == 1, );
#endif
    
    _client.run(_url, _params, _headers, _config);
}

void Watchdog::_clientError (const libencloud::Error &error)
{
    LIBENCLOUD_UNUSED(error);

    if (_state == DownState)
        return;

    _state = DownState;
    emit down();
}

void Watchdog::_clientComplete (const QString &response)
{
    LIBENCLOUD_UNUSED(response);

    if (_state == UpState)
        return;

    _state = UpState;
    emit up();
}

}  // namespace libencloud
