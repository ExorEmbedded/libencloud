#include <encloud/Api/CommonApi>
#include <common/common.h>
#include <common/config.h>

#define LIBENCLOUD_API_SETTINGS_TOUT   1000

namespace libencloud {

//
// shared data
//

QSettings *Api::__settings = NULL;
QUrl Api::__url = QUrl(LIBENCLOUD_API_URL);

//
// public methods
//

Api::Api ()
{
    LIBENCLOUD_TRACE;

    _client.setDebug(false);
}

Api::~Api ()
{
    LIBENCLOUD_DELETE(__settings);
}

int Api::init ()
{
    if (__settings != NULL)
        return 0;

    // QCC needs to wait for encloud port settings;
    // other products use fixed port
#ifdef LIBENCLOUD_MODE_QCC
    LIBENCLOUD_DBG("[Api] Creating shared Settings object");

    // API settings have System Scope (used by Encloud Service)
    __settings = new QSettings(QSettings::SystemScope, LIBENCLOUD_ORG, LIBENCLOUD_APP);
    LIBENCLOUD_DBG("[Api] file: " << __settings->fileName());
    LIBENCLOUD_ERR_IF (__settings == NULL);

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
    __url.setPort(port);
}

//
// static methods
// 

const QSettings *Api::getSettings ()     { return __settings; };

const QUrl &Api::getUrl ()               { return __url; };

//
// protected slots
// 

void Api::_settingsTimeout ()
{
    if (!__settings->contains("port"))
    {
        LIBENCLOUD_DBG("[Api] Waiting for Encloud Settings");
    }
    else 
    {
        _settingsTimer.stop();

        int port = __settings->value("port").toInt();
        __url.setPort(port);

        LIBENCLOUD_DBG("[Api] Settings found - url: " << __url.toString());

        emit gotSettings();
    }
}

} // namespace libencloud
