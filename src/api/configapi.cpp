#include <encloud/Json>
#include <encloud/Api/ConfigApi>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

ConfigApi::ConfigApi ()
{
    LIBENCLOUD_TRACE;

    connect(&_client, SIGNAL(error(libencloud::Error)), this, SLOT(_error(libencloud::Error)));
    connect(&_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>)), this, SLOT(_clientComplete(QString)));
}

ConfigApi::~ConfigApi ()
{
    LIBENCLOUD_TRACE;
}

int ConfigApi::configSupply (const QVariant &config)
{
    QUrl url(getUrl());
	bool ok;

    url.setPath(LIBENCLOUD_API_CONFIG_PATH);

	_headers["Content-Type"] = "application/json";

	QString js = json::serialize(config, ok);
	LIBENCLOUD_ERR_IF (!ok);

    LIBENCLOUD_DBG("[ConfigApi] config url: " << url.toString() << ", data: " << js);

    _client.post(url, _headers, js.toAscii(), _config);

	return 0;
err:
	return ~0;
}

void ConfigApi::_error (const libencloud::Error &err)
{
    LIBENCLOUD_DBG("[ConfigApi] error: " << err.toString());

    emit configSent(Api::ErrorRc);
}

void ConfigApi::_clientComplete (const QString &response)
{
    LIBENCLOUD_UNUSED(response);

    emit configSent(Api::SuccessRc);
}

} // namespace libencloud
