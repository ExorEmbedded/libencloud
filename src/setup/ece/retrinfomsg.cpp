#include <common/common.h>
#include <common/config.h>
#include <encloud/Utils>
#include <setup/ece/ececommon.h>
#include <setup/ece/retrinfomsg.h>

namespace libencloud {

//
// public methods
//

//
// public slots
//

int RetrInfoMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QSslConfiguration config;

    EMIT_ERROR_ERR_IF (_cfg == NULL);

    // client already destroyed via deleteLater()
    //LIBENCLOUD_DELETE (_client);
    LIBENCLOUD_ERR_IF ((_client = new Client) == NULL);

    EMIT_ERROR_ERR_IF (setupece::loadSslConfig(setupece::ProtocolTypeInit, _cfg, url, config));

    switch (_packRequest())
    {
        case 0:  // ok - continue on
            break;
        case 1:  // data needed - don't emit errors here otherwise ecesetup
                 // will keep on retrying - data must be enered by user first
            goto err;
        default:  // otherwise emit a nonsilent error
            EMIT_ERROR_ERR_IF (1);
    }

    EMIT_ERROR_ERR_IF (_encodeRequest(url, params));

    // setup signals from client
    connect(_client, SIGNAL(error(libencloud::Error)), this, SIGNAL(error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, const QMap<QByteArray, QByteArray> &headers)), this, SLOT(_clientComplete(QString)));

    _client->run(url, params, QMap<QByteArray, QByteArray>(), config);

    return 0;
err:
    LIBENCLOUD_DELETE(_client);
    return ~0;
}

#ifdef LIBENCLOUD_MODE_SECE
void RetrInfoMsg::licenseReceived (const QUuid &uuid)
{
    QString license = uuid.toString();

    LIBENCLOUD_DBG("uuid: " << license);

    _cfg->sysSettings->setValue("lic", license);
    _cfg->sysSettings->sync();
    LIBENCLOUD_ERR_MSG_IF (_cfg->sysSettings->status() != QSettings::NoError, 
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
    // signals are emitted internally
    LIBENCLOUD_ERR_IF (_decodeResponse(response));
    LIBENCLOUD_ERR_IF (_unpackResponse());

    emit processed();
err:
    sender()->deleteLater();
    return;
}

//
// private methods
//

int RetrInfoMsg::_packRequest ()
{
#ifdef LIBENCLOUD_MODE_SECE
    _license = QUuid(_cfg->sysSettings->value("lic").toString());

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
    params.addQueryItem("lic", utils::uuid2String(_license));
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
    EMIT_ERROR_ERR_IF (!ok || jo.isEmpty());

    // <TEST> failure
#if 0
    LIBENCLOUD_ERR_IF (1);
#endif

    errString = jo["error"].toString();

    _valid = jo["valid"].toBool();
    if (!_valid)
    {
        if (!errString.isEmpty())
            LIBENCLOUD_ERR("SB error: " + errString);

        LIBENCLOUD_EMIT (error(Error::CodeServerLicenseInvalid));

#ifdef LIBENCLOUD_MODE_SECE
        // user intervention required for license entry (see setup API)
        emit need("license");
#endif
        goto err;
    }

    // generic Switchboard Error
    if (!errString.isEmpty())
    {
        LIBENCLOUD_EMIT (error(Error("SB error: " + errString)));
        goto err;
    }

    _time = utils::pytime2DateTime(jo["time"].toString());
    EMIT_ERROR_ERR_IF (!_time.isValid());

    _expiry = utils::pytime2DateTime(jo["expiry"].toString());
    EMIT_ERROR_ERR_IF (!_expiry.isValid());

    _csrTmpl = jo["csr_template"];
    EMIT_ERROR_ERR_IF (_csrTmpl.isNull());

    _caCert = QSslCertificate(jo["ca_cert"].toString().toAscii());
    EMIT_ERROR_ERR_IF (!_caCert.isValid());

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
    LIBENCLOUD_EMIT_ERR_IF (
            (!csrf.open(QIODevice::WriteOnly) ||
            csrf.write(json::serialize(_csrTmpl, ok).toAscii()) == -1),
            error(Error(Error::CodeSystemError, tr("Failed writing CSR template: ") +
                    csrf.errorString())));
    csrf.close();

    // save the Operation CA certificate to file
    LIBENCLOUD_EMIT_ERR_IF (
            (!caf.open(QIODevice::WriteOnly) ||
             caf.write(_caCert.toPem()) == -1),
            error(Error(Error::CodeSystemError, tr("Failed writing CA cert: ") +
                    caf.errorString())));
    caf.close();

    return 0;
err:
    csrf.close();
    caf.close();

    return ~0;
}

} // namespace libencloud
