#include <common/common.h>
#include <common/config.h>
#include <common/utils.h>
#include <setup/ece/ececommon.h>
#include <setup/ece/retrinfomsg.h>

// use only to wrap upper-level methods, otherwise duplicates will be emitted
#define EMIT_ERROR_ERR_IF(cond) LIBENCLOUD_EMIT_ERR_IF(cond, error())

namespace libencloud {

//
// public methods
//

//
// public slots
//

void RetrInfoMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QSslConfiguration config;

    EMIT_ERROR_ERR_IF (_cfg == NULL);
    EMIT_ERROR_ERR_IF (_client == NULL);

    EMIT_ERROR_ERR_IF (setupece::loadSslConfig(setupece::ProtocolTypeInit, _cfg, url, config));

    switch (_packRequest())
    {
        case 0:  // ok - continue on
            break;
        case 1:  // data needed - emit a silent error
            emit error();
            goto err;
        default:  // otherwise emit a nonsilent error
            EMIT_ERROR_ERR_IF (1);
    }

    EMIT_ERROR_ERR_IF (_encodeRequest(url, params));

    // setup signals from client
    disconnect(_client, 0, this, 0);
    connect(_client, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    _client->run(url, params, QMap<QByteArray, QByteArray>(), config);

err:
    return;
}

#ifdef LIBENCLOUD_MODE_SECE
void RetrInfoMsg::licenseReceived (QUuid uuid)
{
    QString license = uuid.toString();

    LIBENCLOUD_DBG("uuid: " << license);

    _cfg->settings->setValue("lic", license);
    _cfg->settings->sync();
    LIBENCLOUD_ERR_MSG_IF (_cfg->settings->status() != QSettings::NoError, 
            "could not write configuration to file - check permissions!");
err:
    return;
}
#endif

//
// private slots
// 
void RetrInfoMsg::_clientComplete (const QString &response)
{
    // stop listening to signals from client
    disconnect(_client, 0, this, 0);

    EMIT_ERROR_ERR_IF (_decodeResponse(response));
    EMIT_ERROR_ERR_IF (_unpackResponse());

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

    if (_license.isNull()) 
    { 
        emit need("license");
        return 1;
    }

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
    LIBENCLOUD_ERR_IF (!ok || jo.isEmpty());

    // <TEST> failure
#if 0
    LIBENCLOUD_ERR_IF (1);
#endif

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
    LIBENCLOUD_ERR_IF (_csrTmpl.isNull());

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
