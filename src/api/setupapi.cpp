#include <encloud/Api/Setup>
#include <common/common.h>
#include <common/config.h>
#include <api/api.h>

namespace libencloud {

//
// public methods
//

SetupApi::SetupApi ()
    : _msgType(NoneType)
{
    LIBENCLOUD_TRACE;

    connect(&_client, SIGNAL(error(QString)), this, SLOT(_error(QString)));
    connect(&_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    _url.setPath(LIBENCLOUD_API_SETUP_PATH);
}

SetupApi::~SetupApi ()
{
    LIBENCLOUD_TRACE;
}

void SetupApi::poiRetrieve ()
{
    _msgType = PoiRetrieveType;

    _params.clear();

    _client.run(_url, _params, _headers, _config);
}

void SetupApi::licenseSupply (const QUuid &uuid)
{
    _msgType = LicenseSupplyType;

    _params.clear();
    _params.addQueryItem("license", uuid.toString());

    _client.run(_url, _params, _headers, _config);
}

void SetupApi::portSupply (int port)
{
    _msgType = PortSupplyType;

    _params.clear();
    _params.addQueryItem("clientPort", QString::number(port));

    _client.run(_url, _params, _headers, _config);
}

void SetupApi::configRetrieve ()
{
    _msgType = ConfigRetrieveType;

    _params.clear();

    _client.run(_url, _params, _headers, _config);
}

void SetupApi::_error (const QString &err)
{
    LIBENCLOUD_DBG("err: " << err);

    switch (_msgType)
    {
        case PoiRetrieveType:
            emit poiReceived(Api::ErrorRc);
            break;
        case LicenseSupplyType:
            emit licenseSent(Api::ErrorRc);
            break;
        case PortSupplyType:
            emit portSent(Api::ErrorRc);
            break;
        case ConfigRetrieveType:
            emit configReceived(Api::ErrorRc, QVariant());
            break;
        default:
            break;
    }
}

void SetupApi::_clientComplete (const QString &response)
{
    LIBENCLOUD_DBG("response: " << response);

    switch (_msgType)
    {
        case PoiRetrieveType:
        {
            bool ok;
            QVariantMap jo = json::parseJsonp(response, ok).toMap();

            LIBENCLOUD_ERR_IF (!ok);
            LIBENCLOUD_ERR_IF (jo["poi"].isNull());

            emit poiReceived(Api::SuccessRc, QUuid(jo["poi"].toString()));
            break;
        }
        case LicenseSupplyType:
            emit licenseSent(Api::SuccessRc);
            break;
        case PortSupplyType:
            emit portSent(Api::SuccessRc);
            break;
        case ConfigRetrieveType:
        {
            bool ok;

            QVariant config = json::parseJsonp(response, ok);
            LIBENCLOUD_ERR_IF (!ok);

            emit configReceived(Api::SuccessRc, config);
            break;
        }
        default:
            break;
    }

err:
    return;
}

} // namespace libencloud
