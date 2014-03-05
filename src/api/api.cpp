#include <encloud/Api/Common>
#include <common/common.h>
#include <common/config.h>
#include <api/api.h>

#define LIBENCLOUD_API_SETTINGS_TOUT   1000

namespace libencloud {

QSettings *Api::_settings = NULL;

//
// public methods
//

Api::Api ()
{
    LIBENCLOUD_TRACE;

    _url.setScheme(LIBENCLOUD_API_SCHEME);
    _url.setHost(LIBENCLOUD_API_HOST);
    _url.setPort(LIBENCLOUD_API_PORT);

    _client.setDebug(false);
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
    LIBENCLOUD_ERR_IF (0);  // avoid unused label

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
    _url.setPort(port);
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
        _url.setPort(_settings->value("port").toInt());
        emit gotSettings();
    }
}

//
// static methods
// 

} // namespace libencloud
