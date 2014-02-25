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

libencloud::Message::Message()
{
}

/**
 * MessageRetrInfo
 */

libencloud::MessageRetrInfo::MessageRetrInfo ()
{
}

libencloud_rc libencloud::MessageRetrInfo::encodeRequest (QUrl &url, QUrl &params)
{
    url.setPath(LIBENCLOUD_CMD_GETINFO);

#ifdef LIBENCLOUD_MODE_SECE
    params.addQueryItem("lic", this->license.toString().remove('{').remove('}'));
    params.addQueryItem("hw_info", this->hwInfo);
#endif

    LIBENCLOUD_DBG(" ### >>>>> ### " << params.toString());

    return LIBENCLOUD_RC_SUCCESS;
}

libencloud_rc libencloud::MessageRetrInfo::decodeResponse (QString &response, QString &errString)
{ 
    bool ok;
    QVariantMap jo = libencloud::json::parse(response, ok).toMap();
    LIBENCLOUD_ERR_IF (!ok);

    errString = jo["error"].toString();
    LIBENCLOUD_ERR_IF (!errString.isEmpty());

    this->valid = jo["valid"].toBool();

    this->expiry = libencloud::utils::pytime2DateTime(jo["expiry"].toString());
    LIBENCLOUD_ERR_IF (!this->expiry.isValid());

    this->time = libencloud::utils::pytime2DateTime(jo["time"].toString());
    LIBENCLOUD_ERR_IF (!this->time.isValid());

    this->csrTmpl = jo["csr_template"];

    this->caCert = QSslCertificate(jo["ca_cert"].toString().toAscii());
    LIBENCLOUD_ERR_IF (!this->caCert.isValid());

    return LIBENCLOUD_RC_SUCCESS;
err:
    return LIBENCLOUD_RC_BADRESPONSE;
}

/**
 * MessageRetrCert
 */

libencloud::MessageRetrCert::MessageRetrCert ()
{
}

libencloud_rc libencloud::MessageRetrCert::encodeRequest (QUrl &url, QUrl &params)
{
    url.setPath(LIBENCLOUD_CMD_GETCERT);

#ifdef LIBENCLOUD_MODE_SECE
    params.addQueryItem("lic", this->license.toString().remove('{').remove('}'));
    params.addQueryItem("hw_info", this->hwInfo);
#endif

    params.addQueryItem("certificate_request_data", this->csr);

    LIBENCLOUD_DBG(" ### >>>>> ### " << params.toString());

    return LIBENCLOUD_RC_SUCCESS;
}

libencloud_rc libencloud::MessageRetrCert::decodeResponse (QString &response, QString &errString)
{ 
    bool ok;
    QVariantMap jo = libencloud::json::parse(response, ok).toMap();
    LIBENCLOUD_ERR_IF (!ok);

    errString = jo["error"].toString();
    LIBENCLOUD_ERR_IF (!errString.isEmpty());

    this->cert = QSslCertificate(jo["certificate"].toString().toAscii());
    // "checks that the current data-time is within the date-time range during
    // which the certificate is considered valid, and checks that the
    // certificate is not in a blacklist of fraudulent certificates"
    LIBENCLOUD_RETURN_IF (!this->cert.isValid(), LIBENCLOUD_RC_INVALIDCERT);

    this->time = libencloud::utils::pytime2DateTime(jo["time"].toString());
    LIBENCLOUD_ERR_IF (!this->time.isValid());

    return LIBENCLOUD_RC_SUCCESS;
err:
    return LIBENCLOUD_RC_BADRESPONSE;
}

/**
 * MessageRetrConf
 */

libencloud::MessageRetrConf::MessageRetrConf ()
{
}

libencloud_rc libencloud::MessageRetrConf::encodeRequest (QUrl &url, QUrl &params)
{ 
    const QUrl *p = &params;  // unused
    LIBENCLOUD_UNUSED(p);

    url.setPath(LIBENCLOUD_CMD_GETCONFIG);

    LIBENCLOUD_DBG(" ### >>>>> ### GET (no params)");

    return LIBENCLOUD_RC_SUCCESS;
}

libencloud_rc libencloud::MessageRetrConf::decodeResponse (QString &response, QString &errString)
{
    bool ok;
    QVariantMap jo = libencloud::json::parse(response, ok).toMap()["vpn"].toMap();
    LIBENCLOUD_ERR_IF (!ok);

    errString = jo["error"].toString();
    LIBENCLOUD_ERR_IF (!errString.isEmpty());

    this->vpnIp = jo["ip"].toString();
    LIBENCLOUD_ERR_IF (this->vpnIp.isEmpty());

    this->vpnPort = jo["port"].toInt();
    LIBENCLOUD_ERR_IF (this->vpnPort <= 0);

    this->vpnProto = jo["proto"].toString();
    LIBENCLOUD_ERR_IF (this->vpnProto.isEmpty());

    this->vpnType = jo["type"].toString();
    LIBENCLOUD_ERR_IF (this->vpnType.isEmpty());

    this->time = libencloud::utils::pytime2DateTime(jo["time"].toString());
    LIBENCLOUD_ERR_IF (!this->time.isValid());

    return LIBENCLOUD_RC_SUCCESS;
err:
    return LIBENCLOUD_RC_BADRESPONSE;
}
