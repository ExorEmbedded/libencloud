#include <QStringList>
#include "msg.h"
#include "json.h"
#include "helpers.h"
#include "defaults.h"
#include "utils.h"

/**
 * Message
 */

Ece::Message::Message()
{
}

/**
 * MessageRetrInfo
 */

Ece::MessageRetrInfo::MessageRetrInfo ()
{
}

ece_rc_t Ece::MessageRetrInfo::encodeRequest (QUrl &url, QUrl &params)
{
    QtJson::JsonObject jo;
    QString js;

    url.setPath(ECE_CMD_GETINFO);

    jo["lic"] = this->license.toString();
    jo["hw_info"] = this->hwInfo;

    js = QtJson::serialize(jo);
    ECE_ERR_IF (js.isEmpty());
    
    ECE_DBG(" ### >>>>> ### " << js);

    params.addQueryItem(ECE_MSG_PARAM_DEFAULT, js);

    return ECE_RC_SUCCESS;
err:
    return ECE_RC_GENERIC;
}

ece_rc_t Ece::MessageRetrInfo::decodeResponse (QString &response)
{ 
    bool ok;
    QtJson::JsonObject jo = QtJson::parse(response, ok).toMap();
    ECE_ERR_IF (!ok);

    this->valid = jo["valid"].toBool();

    this->expiry = EceUtils::pytime2DateTime(jo["expiry"].toString());
    ECE_ERR_IF (!this->expiry.isValid());

    this->time = EceUtils::pytime2DateTime(jo["time"].toString());
    ECE_ERR_IF (!this->time.isValid());

    this->csr_tmpl = jo["csr_tmpl"];

    return ECE_RC_SUCCESS;
err:
    return ECE_RC_GENERIC;
}

/**
 * MessageRetrCert
 */

Ece::MessageRetrCert::MessageRetrCert ()
{
}

ece_rc_t Ece::MessageRetrCert::encodeRequest (QUrl &url, QUrl &params)
{
    QtJson::JsonObject jo;
    QString js;

    url.setPath(ECE_CMD_GETCERT);

    jo["lic"] = this->license.toString();
    jo["hw_info"] = this->hwInfo;
    jo["certificate_request_data"] = this->csr;

    js = QtJson::serialize(jo);
    ECE_ERR_IF (js.isEmpty());
    
    ECE_DBG(" ### >>>>> ### " << js);

    params.addQueryItem(ECE_MSG_PARAM_DEFAULT, js);

    return ECE_RC_SUCCESS;
err:
    return ECE_RC_GENERIC;
}

ece_rc_t Ece::MessageRetrCert::decodeResponse (QString &response)
{ 
    bool ok;
    QtJson::JsonObject jo = QtJson::parse(response, ok).toMap();
    ECE_ERR_IF (!ok);

    this->cert = QSslCertificate(jo["certificate"].toString().toAscii());
    ECE_ERR_IF (!this->cert.isValid());

    this->time = EceUtils::pytime2DateTime(jo["time"].toString());
    ECE_ERR_IF (!this->time.isValid());

    return ECE_RC_SUCCESS;
err:
    return ECE_RC_GENERIC;
}

/**
 * MessageRetrConf
 */

Ece::MessageRetrConf::MessageRetrConf ()
{
}

ece_rc_t Ece::MessageRetrConf::encodeRequest (QUrl &url, QUrl &params)
{ 
    const QUrl *p = &params;  // unused
    ECE_UNUSED(p);

    url.setPath(ECE_CMD_GETCONFIG);

    ECE_DBG(" ### >>>>> ### PLAIN GET!!! (no Json)");

    return ECE_RC_SUCCESS;
}

ece_rc_t Ece::MessageRetrConf::decodeResponse (QString &response)
{
    bool ok;
    QtJson::JsonObject jo = QtJson::parse(response, ok).toMap()["vpn"].toMap();
    ECE_ERR_IF (!ok);

    this->vpnIp = jo["ip"].toString();
    ECE_ERR_IF (this->vpnIp.isEmpty());

    this->vpnPort = jo["port"].toInt();
    ECE_ERR_IF (this->vpnPort <= 0);

    this->vpnProto = jo["proto"].toString();
    ECE_ERR_IF (this->vpnProto.isEmpty());

    this->vpnType = jo["type"].toString();
    ECE_ERR_IF (this->vpnType.isEmpty());

    this->time = EceUtils::pytime2DateTime(jo["time"].toString());
    ECE_ERR_IF (!this->time.isValid());

    return ECE_RC_SUCCESS;
err:
    return ECE_RC_GENERIC;
}
