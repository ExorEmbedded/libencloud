#include <encloud/Json>
#include <encloud/Api/ConfigApi>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

ConfigApi::ConfigApi ()
    : _msgType(NoneType)
{
    LIBENCLOUD_TRACE;

    connect(&_client, SIGNAL(error(libencloud::Error)), this, SLOT(_error(libencloud::Error)));
    connect(&_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>)), this, SLOT(_clientComplete(QString)));
}

ConfigApi::~ConfigApi ()
{
    LIBENCLOUD_TRACE;
}

void ConfigApi::configRetrieve ()
{
    QUrl url(getUrl());

    url.setPath(LIBENCLOUD_API_CONFIG_PATH);
    _params.clear();

    _msgType = ConfigRetrieveType;

    LIBENCLOUD_DBG("[ConfigApi] config url: " << url.toString());

    _client.run(url, _params, _headers, _config);
}

int ConfigApi::configSupply (const QVariant &config)
{
    QUrl url(getUrl());
	bool ok;

    url.setPath(LIBENCLOUD_API_CONFIG_PATH);

	_headers["Content-Type"] = "application/json";

	QString js = json::serialize(config, ok);
	LIBENCLOUD_ERR_IF (!ok);

    _msgType = ConfigSupplyType;

    LIBENCLOUD_DBG("[ConfigApi] config url: " << url.toString() << ", data: " << js);

    _client.post(url, _headers, js.toAscii(), _config);

	return 0;
err:
	return ~0;
}

void ConfigApi::_error (const libencloud::Error &err)
{
    LIBENCLOUD_DBG("[ConfigApi] error: " << err.toString());

    switch (_msgType)
    {
        case NoneType:
            break;
        case ConfigRetrieveType:
            emit configReceived(Api::ErrorRc, QVariant());
            break;
        case ConfigSupplyType:
            emit configSent(Api::ErrorRc);
            break;
    }
}

void ConfigApi::_clientComplete (const QString &response)
{
    switch (_msgType)
    {
        case NoneType:
            break;
        case ConfigRetrieveType:
        {
            bool ok;
            QVariant jo = json::parse(response, ok);
            //LIBENCLOUD_ERR_IF (!ok);
            if (!ok)
                return;

            emit configReceived(Api::SuccessRc, jo);
            break;
        }
        case ConfigSupplyType:
            emit configSent(Api::SuccessRc);
            break;
    }
}

} // namespace libencloud
