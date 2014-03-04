#include <encloud/Api/Auth>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

AuthApi::AuthApi ()
{
    LIBENCLOUD_TRACE;
}

AuthApi::~AuthApi ()
{
    LIBENCLOUD_TRACE;

    _client.setDebug(false);
    connect(&_client, SIGNAL(error(QString)), this, SLOT(_error(QString)));
    connect(&_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));
}

//
// public slots
//

void AuthApi::authSupply (const Auth &auth)
{
    LIBENCLOUD_DBG("user: " << auth.getUser());

    QUrl url(QString(LIBENCLOUD_API_SCHEME) + _host +
            QString(':') + QString::number(_port));
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration config;

    url.setPath(LIBENCLOUD_API_AUTH_PATH);

    params.addQueryItem("id", auth.getStrId());
    params.addQueryItem("type", auth.getStrType());
    params.addQueryItem("url", auth.getUrl());
    params.addQueryItem("user", auth.getUser());
    params.addQueryItem("pass", auth.getPass());

    _client.run(url, params, headers, config);
}

//
// protected slots
//

void AuthApi::_error (const QString &err)
{
    LIBENCLOUD_DBG("err: " << err);

    emit completed(Api::ErrorRc);
}

void AuthApi::_clientComplete (const QString &response)
{
    LIBENCLOUD_DBG("response: " << response);

    emit completed(Api::SuccessRc);
}

} // namespace libencloud
