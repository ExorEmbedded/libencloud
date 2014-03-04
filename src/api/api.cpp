#include <encloud/Api/Common>
#include <common/common.h>
#include <common/config.h>

#define LIBENCLOUD_API_SETTINGS_TOUT   1000

namespace libencloud {

QSettings *Api::_settings = NULL;

//
// public methods
//

Api::Api ()
    : _host(LIBENCLOUD_API_HOST)
    , _port(LIBENCLOUD_API_PORT)
{
    LIBENCLOUD_TRACE;
}

Api::~Api ()
{
    LIBENCLOUD_DELETE(_settings);
}

int Api::init ()
{
    if (_settings != NULL)
        return 0;

    // QIC needs to wait for encloud port settings;
    // other products use fixed port
#ifdef LIBENCLOUD_MODE_QIC
    _settings = new QSettings(LIBENCLOUD_ORG, LIBENCLOUD_APP);
    LIBENCLOUD_ERR_IF (_settings == NULL);

    _settingsTimer.start(LIBENCLOUD_API_SETTINGS_TOUT);
    _settingsTimeout();
    connect(&_settingsTimer, SIGNAL(timeout()), this, SLOT(_settingsTimeout()));
#endif

    return 0;
err:
    return ~0;
}

void Api::setHost (const QString &host)
{
    _host = host;
}

void Api::setPort (int port)
{
    _port = port;
}

//
// protected slots
// 

void Api::_settingsTimeout ()
{
    if (!_settings->contains("port"))
    {
        LIBENCLOUD_DBG("Waiting for Encloud Settings");
    }
    else 
    {
        LIBENCLOUD_DBG("Settings found");

        _settingsTimer.stop();
        _port = _settings->value("port").toInt();
        emit gotSettings();
    }
}

//
// static methods
// 

} // namespace libencloud
