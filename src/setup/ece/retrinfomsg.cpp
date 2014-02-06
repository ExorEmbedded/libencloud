#include <common/common.h>
#include <common/config.h>
#include <common/utils.h>
#include <setup/ece/common.h>
#include <setup/ece/retrinfomsg.h>

// use only to wrap upper-level methods, otherwise duplicates will be emitted
#define SIGNAL_ERR_IF(cond) LIBENCLOUD_EMIT_ERR_IF(cond, error())

namespace libencloud {

//
// public methods
//

int RetrInfoMsg::init ()
{
    LIBENCLOUD_TRACE;

    connect(_client, SIGNAL(error()), this, SIGNAL(error()));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    return 0;
}

//
// public slots
//

void RetrInfoMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QSslConfiguration config;

    SIGNAL_ERR_IF (_cfg == NULL);
    SIGNAL_ERR_IF (_client == NULL);

    SIGNAL_ERR_IF (setupece::loadSslConfig(setupece::ProtocolTypeInit, _cfg, url, config));

    SIGNAL_ERR_IF (_packRequest());
    SIGNAL_ERR_IF (_encodeRequest(url, params));

    _client->run(url, params, config);

err:
    return;
}

//
// private slots
// 
void RetrInfoMsg::_clientComplete (const QString &response)
{
    SIGNAL_ERR_IF (_decodeResponse(response));
    SIGNAL_ERR_IF (_unpackResponse());

    emit processed();
err:
    return;
}

//
// private methods
// 

int RetrInfoMsg::_packRequest ()
{
#ifdef LIBENCLOUD_MODE_SECE
    _license = QUuid(_cfg->settings->value("lic").toString());
    LIBENCLOUD_RETURN_IF (_license.isNull(), ~0);

    _hwInfo = utils::getHwInfo();
#endif
    return 0;
}

int RetrInfoMsg::_encodeRequest (QUrl &url, QUrl &params)
{
    url.setPath(LIBENCLOUD_CMD_GETINFO);

#ifdef LIBENCLOUD_MODE_SECE
    params.addQueryItem("lic", _license.toString().remove('{').remove('}'));
    params.addQueryItem("hw_info", _hwInfo);
#else
    LIBENCLOUD_UNUSED(params);
#endif

    return 0;
}

int RetrInfoMsg::_decodeResponse (const QString &response)
{
    bool ok;
    QString errString;

    QVariantMap jo = json::parse(response, ok).toMap();
    LIBENCLOUD_ERR_IF (!ok);

    errString = jo["error"].toString();
    if (!errString.isEmpty())
    {
        LIBENCLOUD_DBG ("SB error: " << errString);
        goto err;
    }

    _time = utils::pytime2DateTime(jo["time"].toString());
    LIBENCLOUD_ERR_IF (!_time.isValid());

    _valid = jo["valid"].toBool();

    _expiry = utils::pytime2DateTime(jo["expiry"].toString());
    LIBENCLOUD_ERR_IF (!_expiry.isValid());

    _csrTmpl = jo["csr_template"];

    _caCert = QSslCertificate(jo["ca_cert"].toString().toAscii());
    LIBENCLOUD_ERR_IF (!_caCert.isValid());

    return 0;
err:
    return ~0;
}

int RetrInfoMsg::_unpackResponse ()
{
    QString csrfn = _cfg->config.csrTmplPath.absoluteFilePath();
    QFile csrf(csrfn);
    QString cafn = _cfg->config.sslOp.caPath.absoluteFilePath();
    QFile caf(cafn);
    bool ok;

    // save the CSR template to file
    LIBENCLOUD_ERR_IF (!csrf.open(QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (csrf.write(json::serialize(_csrTmpl, ok).toAscii()) == -1);
    csrf.close();

    // save the Operation CA certificate to file
    LIBENCLOUD_ERR_IF (!caf.open(QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (caf.write(_caCert.toPem()) == -1);
    caf.close();

    return 0;
err:
    csrf.close();
    caf.close();

    return ~0;
}

} // namespace libencloud
