#include <QStringList>
#include "msg.h"
#include "json.h"
#include "helpers.h"
#include "defaults.h"
#include "utils.h"
#include "config.h"

/**
 * Message
 */

encloud::Message::Message()
{
}

/**
 * MessageRetrInfo
 */

encloud::MessageRetrInfo::MessageRetrInfo ()
{
}

encloud_rc encloud::MessageRetrInfo::encodeRequest (QUrl &url, QUrl &params)
{
    url.setPath(ENCLOUD_CMD_GETINFO);

#ifdef ENCLOUD_TYPE_SECE
    params.addQueryItem("lic", this->license.toString().remove('{').remove('}'));
    params.addQueryItem("hw_info", this->hwInfo);
#endif

    ENCLOUD_DBG(" ### >>>>> ### " << params.toString());

    return ENCLOUD_RC_SUCCESS;
}

encloud_rc encloud::MessageRetrInfo::decodeResponse (QString &response, QString &errString)
{ 
    bool ok;
    QVariantMap jo = encloud::json::parse(response, ok).toMap();
    ENCLOUD_ERR_IF (!ok);

    errString = jo["error"].toString();
    ENCLOUD_ERR_IF (!errString.isEmpty());

    this->valid = jo["valid"].toBool();

    this->expiry = encloud::utils::pytime2DateTime(jo["expiry"].toString());
    ENCLOUD_ERR_IF (!this->expiry.isValid());

    this->time = encloud::utils::pytime2DateTime(jo["time"].toString());
    ENCLOUD_ERR_IF (!this->time.isValid());

    this->csrTmpl = jo["csr_template"];

    this->caCert = QSslCertificate(jo["ca_cert"].toString().toAscii());
    ENCLOUD_ERR_IF (!this->caCert.isValid());

    return ENCLOUD_RC_SUCCESS;
err:
    return ENCLOUD_RC_BADRESPONSE;
}

/**
 * MessageRetrCert
 */

encloud::MessageRetrCert::MessageRetrCert ()
{
}

encloud_rc encloud::MessageRetrCert::encodeRequest (QUrl &url, QUrl &params)
{
    url.setPath(ENCLOUD_CMD_GETCERT);

#ifdef ENCLOUD_TYPE_SECE
    params.addQueryItem("lic", this->license.toString().remove('{').remove('}'));
    params.addQueryItem("hw_info", this->hwInfo);
#endif

    params.addQueryItem("certificate_request_data", this->csr);

    ENCLOUD_DBG(" ### >>>>> ### " << params.toString());

    return ENCLOUD_RC_SUCCESS;
}

encloud_rc encloud::MessageRetrCert::decodeResponse (QString &response, QString &errString)
{ 
    bool ok;
    QVariantMap jo = encloud::json::parse(response, ok).toMap();
    ENCLOUD_ERR_IF (!ok);

    errString = jo["error"].toString();
    ENCLOUD_ERR_IF (!errString.isEmpty());

    this->cert = QSslCertificate(jo["certificate"].toString().toAscii());
    // "checks that the current data-time is within the date-time range during
    // which the certificate is considered valid, and checks that the
    // certificate is not in a blacklist of fraudulent certificates"
    ENCLOUD_RETURN_IF (!this->cert.isValid(), ENCLOUD_RC_INVALIDCERT);

    this->time = encloud::utils::pytime2DateTime(jo["time"].toString());
    ENCLOUD_ERR_IF (!this->time.isValid());

    return ENCLOUD_RC_SUCCESS;
err:
    return ENCLOUD_RC_BADRESPONSE;
}

/**
 * MessageRetrConf
 */

encloud::MessageRetrConf::MessageRetrConf ()
{
}

encloud_rc encloud::MessageRetrConf::encodeRequest (QUrl &url, QUrl &params)
{ 
    const QUrl *p = &params;  // unused
    ENCLOUD_UNUSED(p);

    url.setPath(ENCLOUD_CMD_GETCONFIG);

    ENCLOUD_DBG(" ### >>>>> ### GET (no params)");

    return ENCLOUD_RC_SUCCESS;
}

encloud_rc encloud::MessageRetrConf::decodeResponse (QString &response, QString &errString)
{
    bool ok;
    QVariantMap jo = encloud::json::parse(response, ok).toMap()["vpn"].toMap();
    ENCLOUD_ERR_IF (!ok);

    errString = jo["error"].toString();
    ENCLOUD_ERR_IF (!errString.isEmpty());

    this->vpnIp = jo["ip"].toString();
    ENCLOUD_ERR_IF (this->vpnIp.isEmpty());

    this->vpnPort = jo["port"].toInt();
    ENCLOUD_ERR_IF (this->vpnPort <= 0);

    this->vpnProto = jo["proto"].toString();
    ENCLOUD_ERR_IF (this->vpnProto.isEmpty());

    this->vpnType = jo["type"].toString();
    ENCLOUD_ERR_IF (this->vpnType.isEmpty());

    this->time = encloud::utils::pytime2DateTime(jo["time"].toString());
    ENCLOUD_ERR_IF (!this->time.isValid());

    return ENCLOUD_RC_SUCCESS;
err:
    return ENCLOUD_RC_BADRESPONSE;
}
