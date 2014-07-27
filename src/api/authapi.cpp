#include <encloud/Api/AuthApi>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

AuthApi::AuthApi ()
{
    LIBENCLOUD_TRACE;

    connect(&_client, SIGNAL(error(libencloud::Error)), this, SLOT(_error(libencloud::Error)));
    connect(&_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));
}

AuthApi::~AuthApi ()
{
    LIBENCLOUD_TRACE;
}

//
// public slots
//

void AuthApi::authSupply (const Auth &auth)
{
    QUrl url(Api::getUrl());

    url.setPath(LIBENCLOUD_API_AUTH_PATH);

    LIBENCLOUD_DBG("url: " << url.toString() << 
            ", auth: " << auth.toString());

    _params.clear();
    _params.addQueryItem("id", auth.getStrId());
    _params.addQueryItem("type", auth.getStrType());
    _params.addQueryItem("url", auth.getUrl());
    _params.addQueryItem("user", auth.getUser());
    _params.addQueryItem("pass", auth.getPass());
    _params.addQueryItem("path", auth.getPath());

    _client.run(url, _params, _headers, _config);
}

//
// protected slots
//

void AuthApi::_error (const libencloud::Error &err)
{
    LIBENCLOUD_DBG("err: " << err.toString());

    emit authSent(Api::ErrorRc);
}

void AuthApi::_clientComplete (const QString &response)
{
    LIBENCLOUD_UNUSED(response);

    emit authSent(Api::SuccessRc);
}

} // namespace libencloud
