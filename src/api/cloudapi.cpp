#include <encloud/Api/Cloud>
#include <common/common.h>
#include <common/config.h>
#include <api/api.h>

namespace libencloud {

//
// public methods
//

CloudApi::CloudApi ()
{
    LIBENCLOUD_TRACE;

    connect(&_client, SIGNAL(error(QString)), this, SLOT(_error(QString)));
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

void CloudApi::_error (const QString &err)
{
    LIBENCLOUD_DBG("err: " << err);

    emit actionSent(Api::ErrorRc);
}

void CloudApi::_clientComplete (const QString &response)
{
    LIBENCLOUD_UNUSED(response);

    emit actionSent(Api::SuccessRc);
}

} // namespace libencloud
