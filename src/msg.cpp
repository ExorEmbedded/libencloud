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
    url.setPath(ECE_CMD_GETINFO);

    params.addQueryItem("lic", this->license.toString());
    params.addQueryItem("hw_info", this->hwInfo);

    ECE_DBG(" ### >>>>> ### " << params.toString());

    return ECE_RC_SUCCESS;
}

ece_rc_t Ece::MessageRetrInfo::decodeResponse (QString &response, QString &errString)
{ 
    bool ok;
    QtJson::JsonObject jo = QtJson::parse(response, ok).toMap();
    ECE_ERR_IF (!ok);

    errString = jo["error"].toString();
    ECE_ERR_IF (!errString.isEmpty());

    this->valid = jo["valid"].toBool();

    this->expiry = EceUtils::pytime2DateTime(jo["expiry"].toString());
    ECE_ERR_IF (!this->expiry.isValid());

    this->time = EceUtils::pytime2DateTime(jo["time"].toString());
    ECE_ERR_IF (!this->time.isValid());

    this->csrTmpl = jo["csr_tmpl"];

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
    url.setPath(ECE_CMD_GETCERT);

    params.addQueryItem("lic", this->license.toString());
    params.addQueryItem("hw_info", this->hwInfo);
    params.addQueryItem("certificate_request_data", this->csr);

    ECE_DBG(" ### >>>>> ### " << params.toString());

    return ECE_RC_SUCCESS;
}

ece_rc_t Ece::MessageRetrCert::decodeResponse (QString &response, QString &errString)
{ 
    bool ok;
    QtJson::JsonObject jo = QtJson::parse(response, ok).toMap();
    ECE_ERR_IF (!ok);

    errString = jo["error"].toString();
    ECE_ERR_IF (!errString.isEmpty());

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

    ECE_DBG(" ### >>>>> ### GET (no params)");

    return ECE_RC_SUCCESS;
}

ece_rc_t Ece::MessageRetrConf::decodeResponse (QString &response, QString &errString)
{
    bool ok;
    QtJson::JsonObject jo = QtJson::parse(response, ok).toMap()["vpn"].toMap();
    ECE_ERR_IF (!ok);

    errString = jo["error"].toString();
    ECE_ERR_IF (!errString.isEmpty());

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
