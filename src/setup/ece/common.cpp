#include <QSslKey>
#include <common/common.h>
#include <common/config.h>
#include <setup/ece/common.h>

namespace libencloud {
namespace setupece {

int loadSslConfig (ProtocolType proto, Config *cfg, QUrl &url, QSslConfiguration &sslcfg)
{
    libencloud_config_ssl_t *sslconfig;
    
    LIBENCLOUD_RETURN_IF (!LIBENCLOUD_SETUPECE_PROTOCOLTYPE_VALID(proto), ~0);
    LIBENCLOUD_RETURN_IF (cfg == NULL, ~0);

    switch (proto) {
        case ProtocolTypeInit:
            sslconfig = &cfg->config.sslInit;
            break;
        case ProtocolTypeOp:
            sslconfig = &cfg->config.sslOp;
            break;
        default:
            LIBENCLOUD_RETURN_IF (1, ~0);
    }

    url = sslconfig->sbUrl;
    LIBENCLOUD_DBG("url=" << url.toString());

    // get CA cert(s)
    QSslCertificate ca;
    QList<QSslCertificate> cas(ca.fromPath(sslconfig->caPath.absoluteFilePath()));
    LIBENCLOUD_RETURN_MSG_IF (cas.empty(), ~0, "missing CA cert!");
    LIBENCLOUD_DBG("CaCert subj_CN=" << cas.first().subjectInfo(QSslCertificate::CommonName) << \
            " issuer_O=" << cas.first().issuerInfo(QSslCertificate::Organization));

    // get local cert(s)
    QSslCertificate cert;
    QList<QSslCertificate> certs(cert.fromPath(sslconfig->certPath.absoluteFilePath()));
    LIBENCLOUD_RETURN_MSG_IF (certs.empty(), ~0, "missing cert!");
    LIBENCLOUD_DBG("Cert subj_CN=" << certs.first().subjectInfo(QSslCertificate::CommonName) << \
            " issuer_O=" << certs.first().issuerInfo(QSslCertificate::Organization));

    // get local key
    QFile kfile(sslconfig->keyPath.absoluteFilePath());
    kfile.open(QIODevice::ReadOnly | QIODevice::Text);
    QSslKey key(&kfile, QSsl::Rsa);
    LIBENCLOUD_RETURN_MSG_IF (key.isNull(), ~0, "missing key!");
    LIBENCLOUD_DBG("Key length=" << key.length() << " algorithm=" << key.algorithm());

    sslcfg.setCaCertificates(cas);
    sslcfg.setLocalCertificate(certs.first());
    sslcfg.setPrivateKey(key);

    return 0;
}

} // namespace setupece
} // namespace libencloud
