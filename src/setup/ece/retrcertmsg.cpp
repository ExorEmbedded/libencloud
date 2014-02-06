#include <common/common.h>
#include <common/config.h>
#include <common/utils.h>
#include <setup/ece/common.h>
#include <setup/ece/retrcertmsg.h>

namespace libencloud {

//
// public methods
//

int RetrCertMsg::init ()
{
    LIBENCLOUD_TRACE;

    connect(_client, SIGNAL(error()), this, SIGNAL(error()));
    connect(_client, SIGNAL(complete(QString)), this, SLOT(_clientComplete(QString)));

    return 0;
}

//
// public slots
//

void RetrCertMsg::process ()
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
void RetrCertMsg::_clientComplete (const QString &response)
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

int RetrCertMsg::_packRequest ()
{
    QString keyfn = _cfg->config.sslOp.keyPath.absoluteFilePath();
    QString tmpkeyfn = keyfn + ".tmp";
    char *buf = NULL;
    long len;

#ifdef LIBENCLOUD_MODE_SECE
    _license = QUuid(_cfg->settings->value("lic").toString());
    LIBENCLOUD_ERR_IF (_license.isNull());

    _hwInfo = EceUtils::getHwInfo();
#endif

    // generate temporary key and CSR
    LIBENCLOUD_ERR_IF (libencloud_crypto_genkey(&_cfg->crypto, _cfg->config.rsaBits, qPrintable(tmpkeyfn)));
    LIBENCLOUD_ERR_IF (libencloud_crypto_gencsr(&_cfg->crypto, qPrintable(tmpkeyfn), &buf, &len));

    _csr = QByteArray(buf, len);

    return 0;
err:
    LIBENCLOUD_FREE(buf);
    return ~0;
}

int RetrCertMsg::_encodeRequest (QUrl &url, QUrl &params)
{
    url.setPath(LIBENCLOUD_CMD_GETCERT);

#ifdef LIBENCLOUD_MODE_SECE
    params.addQueryItem("lic", _license.toString().remove('{').remove('}'));
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

    errString = jo["error"].toString();
    if (!errString.isEmpty())
    {
        LIBENCLOUD_DBG ("SB error: " << errString);
        goto err;
    }

    _cert = QSslCertificate(jo["certificate"].toString().toAscii());
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
