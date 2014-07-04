#include <encloud/Api/CloudApi>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

CloudApi::CloudApi ()
{
    LIBENCLOUD_TRACE;

    connect(&_client, SIGNAL(error(libencloud::Error)), this, SLOT(_error(libencloud::Error)));
    connect(&_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));
}

CloudApi::~CloudApi ()
{
    LIBENCLOUD_TRACE;
}

void CloudApi::actionRequest (const QString &action, const Params &params)
{
    QUrl url(getUrl());

    url.setPath(LIBENCLOUD_API_CLOUD_PATH);

    _params.clear();
    _params.addQueryItem("action", action);

    foreach (Param param, params)
        _params.addQueryItem(param.first, param.second);

    LIBENCLOUD_DBG("url: " << url.toString() << ", params: " << _params);

    _client.run(url, _params, _headers, _config);
}

void CloudApi::_error (const libencloud::Error &err)
{
    LIBENCLOUD_DBG("err: " << err.toString());

    emit actionSent(Api::ErrorRc);
}

void CloudApi::_clientComplete (const QString &response)
{
    LIBENCLOUD_UNUSED(response);

    emit actionSent(Api::SuccessRc);
}

} // namespace libencloud
