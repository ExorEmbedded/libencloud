#include <common/common.h>
#include <common/config.h>
#include <encloud/Utils>
#include <setup/ece/ececommon.h>
#include <setup/ece/retrcertmsg.h>

namespace libencloud {

//
// public methods
//

//
// public slots
//

int RetrCertMsg::process ()
{
    LIBENCLOUD_TRACE;

    QUrl url;
    QUrl params;
    QSslConfiguration config;
    const char *certData;

    EMIT_ERROR_ERR_IF (_cfg == NULL);
    LIBENCLOUD_DELETE (_client);
    LIBENCLOUD_ERR_IF ((_client = new Client) == NULL);

    EMIT_ERROR_ERR_IF (setupece::loadSslConfig(setupece::ProtocolTypeInit, _cfg, url, config));

    // don't getCertificate if we already have a valid one
    certData = utils::file2Data(_cfg->config.sslOp.certPath);
    if (certData == NULL)
    {
        LIBENCLOUD_DBG("No existing Operation Certificate");
    }
    else
    {
        if (QSslCertificate(certData).isValid())
        {
            LIBENCLOUD_DBG("Valid Operation Certificate found - skipping getCertificate");
            emit processed();
            return 0;
        }
    }

    EMIT_ERROR_ERR_IF (_packRequest());
    EMIT_ERROR_ERR_IF (_encodeRequest(url, params));

    // listen to signals from client
    disconnect(_client, 0, this, 0);
    connect(_client, SIGNAL(error(libencloud::Error)), this, SIGNAL(error(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    _client->run(url, params, QMap<QByteArray, QByteArray>(), config);

    return 0;
err:
    LIBENCLOUD_DELETE(_client);
    return ~0;
}

//
// private slots
// 
void RetrCertMsg::_clientComplete (const QString &response)
{
    EMIT_ERROR_ERR_IF (_decodeResponse(response));
    EMIT_ERROR_ERR_IF (_unpackResponse());

    emit processed();
err:
    _client->deleteLater();
    return;
}

//
// private methods
// 

int RetrCertMsg::_packRequest ()
{
    QString keyfn = _cfg->config.sslOp.keyPath.absoluteFilePath();
    QString tmpkeyfn = keyfn + ".tmp";
    char *buf = NULL;
    long len;

#ifdef LIBENCLOUD_MODE_SECE
    _license = QUuid(_cfg->sysSettings->value("lic").toString());
    LIBENCLOUD_ERR_IF (_license.isNull());

    _hwInfo = utils::getHwInfo();
#endif

    // generate temporary key and CSR
    LIBENCLOUD_ERR_IF (libencloud_crypto_genkey(&_cfg->crypto, _cfg->config.rsaBits, qPrintable(tmpkeyfn)));
    LIBENCLOUD_ERR_IF (libencloud_crypto_gencsr(&_cfg->crypto, qPrintable(tmpkeyfn), &buf, &len));

    _csr = QByteArray(buf, len);
    LIBENCLOUD_FREE(buf);

    return 0;
err:
    LIBENCLOUD_FREE(buf);
    return ~0;
}

int RetrCertMsg::_encodeRequest (QUrl &url, QUrl &params)
{
    url.setPath(LIBENCLOUD_CMD_GETCERT);

#ifdef LIBENCLOUD_MODE_SECE
    params.addQueryItem("lic", utils::uuid2String(_license));
    params.addQueryItem("hw_info", _hwInfo);
#endif

    params.addQueryItem("certificate_request_data", _csr);

    return 0;
}

int RetrCertMsg::_decodeResponse (const QString &response)
{
    bool ok;
    QString errString;

    QVariantMap jo = json::parse(response, ok).toMap();
    LIBENCLOUD_ERR_IF (!ok);

    // <TEST> failure
#if 0
    LIBENCLOUD_ERR_IF (1);
#endif

    errString = jo["error"].toString();
    if (!errString.isEmpty())
    {
        LIBENCLOUD_ERR ("SB error: " << errString);
        goto err;
    }

    _cert = QSslCertificate(jo["certificate"].toString().toAscii());
    LIBENCLOUD_ERR_IF (_cert.isNull());

    // "checks that the current data-time is within the date-time range during
    // which the certificate is considered valid, and checks that the
    // certificate is not in a blacklist of fraudulent certificates"
    LIBENCLOUD_ERR_IF (!_cert.isValid());

    _time = utils::pytime2DateTime(jo["time"].toString());
    LIBENCLOUD_ERR_IF (!_time.isValid());

    return 0;
err:
    return ~0;
}

int RetrCertMsg::_unpackResponse ()
{
    QString keyfn = _cfg->config.sslOp.keyPath.absoluteFilePath();
    QString tmpkeyfn = keyfn + ".tmp";
    QFile tmpkey(tmpkeyfn);
    QString certfn = _cfg->config.sslOp.certPath.absoluteFilePath();
    QFile certfile(certfn);

    // save the received certificate
    LIBENCLOUD_ERR_IF (!certfile.open(QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (certfile.write(_cert.toPem()) == -1);
    certfile.close();

    // all ok - now we can commit the temporary key
    QFile::remove(keyfn);
    LIBENCLOUD_ERR_IF (!tmpkey.setPermissions(QFile::ReadOwner|QFile::WriteOwner));
    LIBENCLOUD_ERR_IF (!tmpkey.rename(keyfn));

    return 0;
err:
    return ~0;
}

} // namespace libencloud
