#include <QStringList>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <encloud/Common>
#include <encloud/Crypto>
#include <encloud/Utils>
#include "common.h"
#include "config.h"

#undef __LIBENCLOUD_MSG
#define __LIBENCLOUD_MSG __LIBENCLOUD_PRINT

//
// C++ API
//

namespace libencloud {
namespace crypto {

// Set request based on auth
int requestFromAuth (const Auth &auth, QMap<QByteArray,
        QByteArray> &headers, QNetworkRequest &request)
{
    QUrl url;

    QSslConfiguration sslconf = request.sslConfiguration();
    LIBENCLOUD_ERR_IF (libencloud::crypto::configFromAuth(auth, url, headers, sslconf));
    request.setSslConfiguration(sslconf);

    for (QMap<QByteArray, QByteArray>::const_iterator mi = headers.begin(); mi != headers.end(); mi++)
        request.setRawHeader(mi.key(), mi.value());

    return 0;
err:
    return ~0;
}

// Set url, headers and sslconf based on auth
int configFromAuth (const Auth &auth, QUrl &url, QMap<QByteArray,
        QByteArray> &headers, QSslConfiguration &sslconf)
{
    switch (auth.getType())
    {
        case Auth::UserpassType:
        {
            headers["Authorization"] =  crypto::userpassToBasic(
                    auth.getUser(), auth.getPass()).toLocal8Bit();
            break;
        }
        case Auth::CertificateType:
        {
            QString p12Path = auth.getPath();
            QString password = auth.getPass();
            QSslCertificate cert = crypto::p12GetCert(p12Path, password);
            QSslKey key = crypto::p12GetKey(p12Path, password);

            //LIBENCLOUD_DBG("[Crypto] password: " << password);

            LIBENCLOUD_ERR_IF (cert.isNull());
            LIBENCLOUD_ERR_IF (key.isNull());

            sslconf.setLocalCertificate(cert);
            sslconf.setPrivateKey(key);
            break;
        }
        case Auth::CookieType:
        {
            // do nothing - we assume cookie headers are set automatically
            break;
        }
        default:
            goto err;
    }
    
    url.setUrl(auth.getUrl());

    return 0;
err:
    return ~0;
} 

QString userpassToBasic (const QString &user, const QString &pass)
{
    QString authData;
    QString headerData;

    authData = user + ":" + pass;
    headerData = "Basic " + QByteArray(authData.toLocal8Bit().toBase64());

    return headerData;
}

static QString _opensslPath()
{
    return getBinDir() + "/openssl" + LIBENCLOUD_EXE;
}

int certToString (const QString &path, QString &out)
{
    QStringList args;

    args << "x509";
    args << "-text";
    args << "-in" << path;

    return utils::execute(_opensslPath(), args, out, true, false);
}

static int _p12Cmd (const QString &p12Path, const QString &password, const QStringList &extraArgs)
{
    QStringList args;
    QString out;

    args << "pkcs12";
    args << "-nodes";
    args << "-in" << p12Path;
    args << "-passin" << "pass:" + password;

    args.append(extraArgs);

    return utils::execute(_opensslPath(), args, out, true, false);
}

LIBENCLOUD_DLLSPEC QSslCertificate p12GetCa (const QString &p12Path,
        const QString &password)
{
    QTemporaryFile certFile;
    QSslCertificate cert(&certFile);

    LIBENCLOUD_ERR_IF (!certFile.open());
    LIBENCLOUD_ERR_IF (crypto::p12SaveCa(p12Path, password, certFile.fileName()));
    LIBENCLOUD_ERR_IF (cert.isNull());

    return cert;
err:
    return QSslCertificate();
}

int p12SaveCa (const QString &p12Path, const QString &password, const QString &caPath)
{
    QStringList extraArgs;

    extraArgs << "-out" << caPath;
    extraArgs << "-nokeys";
    extraArgs << "-cacerts";

    return (_p12Cmd(p12Path, password, extraArgs));
}

LIBENCLOUD_DLLSPEC QSslCertificate p12GetCert (const QString &p12Path,
        const QString &password)
{
    QTemporaryFile certFile;
    QSslCertificate cert;

    LIBENCLOUD_ERR_IF (!certFile.open());
    LIBENCLOUD_ERR_IF (crypto::p12SaveCert(p12Path, password, certFile.fileName()));

    cert = QSslCertificate(&certFile);
    LIBENCLOUD_ERR_IF (cert.isNull());

    return cert;
err:
    return QSslCertificate();
}

int p12SaveCert (const QString &p12Path, const QString &password, const QString &certPath)
{
    QStringList extraArgs;

    extraArgs << "-out" << certPath;
    extraArgs << "-nokeys";
    extraArgs << "-clcerts";

    return (_p12Cmd(p12Path, password, extraArgs));
}

LIBENCLOUD_DLLSPEC QSslKey p12GetKey (const QString &p12Path,
        const QString &password)
{
    QTemporaryFile keyFile;
    QSslKey key;

    LIBENCLOUD_ERR_IF (!keyFile.open());
    LIBENCLOUD_ERR_IF (crypto::p12SaveKey(p12Path, password, keyFile.fileName()));

    key = QSslKey(&keyFile, QSsl::Rsa);
    LIBENCLOUD_ERR_IF (key.isNull());

    return key;
err:
    return QSslKey();
}

int p12SaveKey (const QString &p12Path, const QString &password, const QString &keyPath)
{
    QStringList extraArgs;

    extraArgs << "-out" << keyPath;
    extraArgs << "-nocerts";

    return (_p12Cmd(p12Path, password, extraArgs));
}

}  // namespace crypto
}  // namespace libencloud

//
// C API
//

static X509_REQ *__make_req (libencloud_crypto_t *ec, EVP_PKEY *pkey);

/** \brief Initialize crypto context */
int libencloud_crypto_init (libencloud_crypto_t *ec)
{
    LIBENCLOUD_TRACE;

    if (ec == NULL)  // context is optional for now
        return 0;

    memset(ec, 0, sizeof(libencloud_crypto_t));

    OpenSSL_add_all_algorithms();

    return 0;
}

/** \brief Release crypto context */
int libencloud_crypto_term (libencloud_crypto_t *ec)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_UNUSED(ec);

    return 0;
}

/** \brief Set callback used by calling API to set X509 subject name values */
int libencloud_crypto_set_name_cb (libencloud_crypto_t *ec, int cb(X509_NAME *n, void *arg), void *ctx)
{
    LIBENCLOUD_ERR_IF (ec == NULL);
    LIBENCLOUD_ERR_IF (cb == NULL);

    ec->name_cb = cb;
    ec->name_cb_ctx = ctx;

    return 0;
err:
    return ~0;
}

/** \brief Generate an RSA key of size 'nbits' to 'outfile' */
int libencloud_crypto_genkey (libencloud_crypto_t *ec, size_t nbits, const char *outfile)
{
    int rc = ~0;
    unsigned long f4 = RSA_F4;
    const EVP_CIPHER *enc = NULL;
    char rbuf[256];
    const char *file = NULL;
    BIGNUM *bn = NULL;
    BIO *out = NULL;
    RSA *rsa = NULL;

    if (nbits == 0)
        nbits = 2048;

    LIBENCLOUD_TRACE;

    LIBENCLOUD_UNUSED(ec);
    LIBENCLOUD_ERR_IF (outfile == NULL);

    // grab name of rand file path - see RAND_load_file(3)
    LIBENCLOUD_ERR_IF ((file = RAND_file_name(rbuf, sizeof(rbuf))) == NULL);

    // the following is allowed to fail if seeding is not based on rand file
    // e.g. /dev/urandom is used if found on system,
    RAND_load_file(file, -1);

    // but still make sure random number generator is adequately seeded
    LIBENCLOUD_ERR_IF (!RAND_status());

    // create output file
    LIBENCLOUD_ERR_IF ((out = BIO_new(BIO_s_file())) == NULL);
    LIBENCLOUD_ERR_IF (BIO_write_filename(out, (char *) outfile) <= 0);

    // run RSA keygen algo
    LIBENCLOUD_ERR_IF ((bn = BN_new()) == NULL);
    LIBENCLOUD_ERR_IF ((rsa = RSA_new()) == NULL);
    LIBENCLOUD_ERR_IF (!BN_set_word(bn, f4));
    LIBENCLOUD_ERR_IF (!RSA_generate_key_ex(rsa, nbits, bn, NULL));
    LIBENCLOUD_ERR_IF (!PEM_write_bio_RSAPrivateKey(out, rsa, enc, NULL, 0, NULL, NULL));

    rc = 0;
err:
    if (rsa)
        RSA_free(rsa);
    if (out)
        BIO_free_all(out);
    if (bn)
        BN_free(bn);

    return rc;
}

/** \brief Write CSR based on 'keyfile' to output buffer {pbuf,plen}
 *  (or to file if pbuf == NULL)
 *  
 *  If pbuf is defined, user owns it and must free() when finished with it.
 *
 *  Prerequisites: libencloud_crypto_set_name_cb() for subject name settings
 */
int libencloud_crypto_gencsr (libencloud_crypto_t *ec, const char *keyfile, char **pbuf, long *plen)
{
    int rc = ~0;
    BIO *kfile = NULL;
    BIO *out = NULL;
    EVP_PKEY *pkey = NULL;
    X509_REQ *req = NULL;
    long errline = -1;
    char *buf = NULL, *pb;
    long len = 0;

    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (keyfile == NULL);
    LIBENCLOUD_ERR_IF (ec == NULL);
    LIBENCLOUD_ERR_IF (ec->name_cb == NULL);

    LIBENCLOUD_ERR_IF ((kfile = BIO_new(BIO_s_file())) == NULL);
    LIBENCLOUD_ERR_IF (BIO_read_filename(kfile, keyfile) <= 0);

    // load private key
    LIBENCLOUD_ERR_IF ((pkey = PEM_read_bio_PrivateKey(kfile, NULL, NULL, NULL)) == NULL);

    // generate the CSR
    LIBENCLOUD_ERR_IF ((req = __make_req(ec, pkey)) == NULL);

    if (pbuf == NULL)  // output to standard output
    {
        LIBENCLOUD_ERR_IF ((out = BIO_new(BIO_s_file())) == NULL);
        BIO_set_fp(out, stdout, BIO_NOCLOSE);

        LIBENCLOUD_ERR_IF (!PEM_write_bio_X509_REQ(out, req));
    }
    else 
    {
        LIBENCLOUD_ERR_IF ((out = BIO_new(BIO_s_mem())) == NULL);

        LIBENCLOUD_ERR_IF (!PEM_write_bio_X509_REQ(out, req));
        BIO_flush(out);

        // memory to buffer
        LIBENCLOUD_ERR_IF ((len = BIO_get_mem_data(out, &pb)) <= 0);
        LIBENCLOUD_ERR_IF ((buf = (char *) calloc(1, sizeof(char) * len)) == NULL);
        memcpy(buf, pb, len);
    }

    if (pbuf)
        *pbuf = buf;
    if (plen) 
        *plen = len;

    rc = 0;
err:
    if (req)
        X509_REQ_free(req);
    if (kfile)
        BIO_free_all(kfile);
    if (out)
        BIO_free_all(out);
    if (rc && buf)
        free(buf);
    if (rc && errline != -1)
        fprintf(stderr, "error at line: %ld\n", errline);

    return rc;
}

static X509_REQ *__make_req (libencloud_crypto_t *ec, EVP_PKEY *pkey)
{
    X509_REQ *req = NULL;
    X509_NAME *n = NULL;
    EVP_MD_CTX mctx;
    EVP_PKEY_CTX *pkctx = NULL;
    const EVP_MD *digest = NULL;

    EVP_MD_CTX_init(&mctx);

    LIBENCLOUD_ERR_IF (ec == NULL);
    LIBENCLOUD_ERR_IF (pkey == NULL);
    LIBENCLOUD_ERR_IF (ec->name_cb == NULL);

    LIBENCLOUD_ERR_IF ((req = X509_REQ_new()) == NULL);
    LIBENCLOUD_ERR_IF ((n = X509_NAME_new()) == NULL);
    
    LIBENCLOUD_ERR_IF (ec->name_cb(n, ec->name_cb_ctx));
    LIBENCLOUD_ERR_IF (!X509_REQ_set_subject_name(req, n));
    LIBENCLOUD_ERR_IF (!X509_REQ_set_pubkey(req, pkey));

    // LIBENCLOUD_ERR_IF ((digest = EVP_get_digestbyname(SN_sha256)) == NULL); Not supported by the current Switchboard openssl version
    LIBENCLOUD_ERR_IF ((digest = EVP_get_digestbyname(SN_sha1)) == NULL);
    LIBENCLOUD_ERR_IF (!EVP_DigestSignInit(&mctx, &pkctx, digest, NULL, pkey));
    LIBENCLOUD_ERR_IF (!X509_REQ_sign_ctx(req, &mctx));
    EVP_MD_CTX_cleanup(&mctx);

    X509_NAME_free(n);

    return req;
err:
    if (n)
        X509_NAME_free(n);
    if (req)
        X509_REQ_free(req);

    return NULL;
}

/** 
 * \brief Calculate md5sum of {buf, buf_sz} and return it as a null-terminated string 
 * 
 * Result string must be free()d by caller.
 */
char *libencloud_crypto_md5 (libencloud_crypto_t *ec, char *buf, long buf_sz)
{
    unsigned char md5[MD5_DIGEST_LENGTH];
    char *s = NULL;
    long i;

    LIBENCLOUD_UNUSED(ec);

    LIBENCLOUD_ERR_IF (buf == NULL);
    LIBENCLOUD_ERR_IF (buf_sz <= 0);

    LIBENCLOUD_ERR_IF (!EVP_Digest(buf, buf_sz, md5, NULL, EVP_md5(), NULL));
    LIBENCLOUD_ERR_IF ((s = (char *) calloc(1, sizeof(char) * MD5_DIGEST_LENGTH*2 + 1)) == NULL);

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(s + (i*2), "%02X", md5[i]);

    s[MD5_DIGEST_LENGTH*2] = '\0';

    return s;
err:
    if (s)
        free(s);

    return NULL;
}
