#include <string.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include "helpers.h"
#include "defaults.h"
// don't depend on Qt for debug
#undef __ENCLOUD_MSG
#define __ENCLOUD_MSG(lev, levstr, msg) __ENCLOUD_PRINT(lev, levstr, msg)
#include "crypto.h"

static X509_REQ *__make_req (encloud_crypto_t *ec, EVP_PKEY *pkey);

/** \brief Initialize crypto context */
int encloud_crypto_init (encloud_crypto_t *ec)
{
    //ENCLOUD_TRACE;

    if (ec == NULL)  // context is optional for now
        return 0;

    memset(ec, 0, sizeof(encloud_crypto_t));

    OpenSSL_add_all_algorithms();

    return 0;
}

/** \brief Release crypto context */
int encloud_crypto_term (encloud_crypto_t *ec)
{
    //ENCLOUD_TRACE;

    ENCLOUD_UNUSED(ec);

    return 0;
}

/** \brief Set callback used by calling API to set X509 subject name values */
int encloud_crypto_set_name_cb (encloud_crypto_t *ec, int cb(X509_NAME *n, void *arg), void *ctx)
{
    ENCLOUD_ERR_IF (ec == NULL);
    ENCLOUD_ERR_IF (cb == NULL);

    ec->name_cb = cb;
    ec->name_cb_ctx = ctx;

    return 0;
err:
    return ~0;
}

/** \brief Generate an RSA key of size 'nbits' to 'outfile' */
int encloud_crypto_genkey (encloud_crypto_t *ec, size_t nbits, const char *outfile)
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

    //ENCLOUD_TRACE;

    ENCLOUD_UNUSED(ec);
    ENCLOUD_ERR_IF (outfile == NULL);

    // grab name of rand file path - see RAND_load_file(3)
    ENCLOUD_ERR_IF ((file = RAND_file_name(rbuf, sizeof(rbuf))) == NULL);

    // the following is allowed to fail if seeding is not based on rand file
    // e.g. /dev/urandom is used if found on system,
    RAND_load_file(file, -1);

    // but still make sure random number generator is adequately seeded
    ENCLOUD_ERR_IF (!RAND_status());

    // create output file
    ENCLOUD_ERR_IF ((out = BIO_new(BIO_s_file())) == NULL);
    ENCLOUD_ERR_IF (BIO_write_filename(out, (char *) outfile) <= 0);

    // run RSA keygen algo
    ENCLOUD_ERR_IF ((bn = BN_new()) == NULL);
    ENCLOUD_ERR_IF ((rsa = RSA_new()) == NULL);
    ENCLOUD_ERR_IF (!BN_set_word(bn, f4));
    ENCLOUD_ERR_IF (!RSA_generate_key_ex(rsa, nbits, bn, NULL));
    ENCLOUD_ERR_IF (!PEM_write_bio_RSAPrivateKey(out, rsa, enc, NULL, 0, NULL, NULL));

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
 *  Prerequisites: encloud_crypto_set_name_cb() for subject name settings
 */
int encloud_crypto_gencsr (encloud_crypto_t *ec, const char *keyfile, char **pbuf, long *plen)
{
    int rc = ~0;
    BIO *kfile = NULL;
    BIO *out = NULL;
    EVP_PKEY *pkey = NULL;
    X509_REQ *req = NULL;
    long errline = -1;
    char *buf = NULL, *pb;
    long len = 0;

    //ENCLOUD_TRACE;

    ENCLOUD_ERR_IF (keyfile == NULL);
    ENCLOUD_ERR_IF (ec == NULL);
    ENCLOUD_ERR_IF (ec->name_cb == NULL);

    ENCLOUD_ERR_IF ((kfile = BIO_new(BIO_s_file())) == NULL);
    ENCLOUD_ERR_IF (BIO_read_filename(kfile, keyfile) <= 0);

    // load private key
    ENCLOUD_ERR_IF ((pkey = PEM_read_bio_PrivateKey(kfile, NULL, NULL, NULL)) == NULL);

    // generate the CSR
    ENCLOUD_ERR_IF ((req = __make_req(ec, pkey)) == NULL);

    if (pbuf == NULL)  // output to standard output
    {
        ENCLOUD_ERR_IF ((out = BIO_new(BIO_s_file())) == NULL);
        BIO_set_fp(out, stdout, BIO_NOCLOSE);

        ENCLOUD_ERR_IF (!PEM_write_bio_X509_REQ(out, req));
    }
    else 
    {
        ENCLOUD_ERR_IF ((out = BIO_new(BIO_s_mem())) == NULL);

        ENCLOUD_ERR_IF (!PEM_write_bio_X509_REQ(out, req));
        BIO_flush(out);

        // memory to buffer
        ENCLOUD_ERR_IF ((len = BIO_get_mem_data(out, &pb)) <= 0);
        ENCLOUD_ERR_IF ((buf = calloc(1, sizeof(char) * len)) == NULL);
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

static X509_REQ *__make_req (encloud_crypto_t *ec, EVP_PKEY *pkey)
{
    X509_REQ *req = NULL;
    X509_NAME *n = NULL;
    EVP_MD_CTX mctx;
    EVP_PKEY_CTX *pkctx = NULL;
    const EVP_MD *digest = NULL;

    EVP_MD_CTX_init(&mctx);

    ENCLOUD_ERR_IF (ec == NULL);
    ENCLOUD_ERR_IF (pkey == NULL);
    ENCLOUD_ERR_IF (ec->name_cb == NULL);

    ENCLOUD_ERR_IF ((req = X509_REQ_new()) == NULL);
    ENCLOUD_ERR_IF ((n = X509_NAME_new()) == NULL);
    
    ENCLOUD_ERR_IF (ec->name_cb(n, ec->name_cb_ctx));
    ENCLOUD_ERR_IF (!X509_REQ_set_subject_name(req, n));
    ENCLOUD_ERR_IF (!X509_REQ_set_pubkey(req, pkey));

    // ENCLOUD_ERR_IF ((digest = EVP_get_digestbyname(SN_sha256)) == NULL); Non supported by the current Switchboard openssl version
    ENCLOUD_ERR_IF ((digest = EVP_get_digestbyname(SN_sha1)) == NULL);
    ENCLOUD_ERR_IF (!EVP_DigestSignInit(&mctx, &pkctx, digest, NULL, pkey));
    ENCLOUD_ERR_IF (!X509_REQ_sign_ctx(req, &mctx));
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
char *encloud_crypto_md5 (encloud_crypto_t *ec, char *buf, long buf_sz)
{
    unsigned char md5[MD5_DIGEST_LENGTH];
    char *s = NULL;
    long i;

    ENCLOUD_UNUSED(ec);

    ENCLOUD_ERR_IF (buf == NULL);
    ENCLOUD_ERR_IF (buf_sz <= 0);

    ENCLOUD_ERR_IF (!EVP_Digest(buf, buf_sz, md5, NULL, EVP_md5(), NULL));
    ENCLOUD_ERR_IF ((s = (char *) calloc(1, sizeof(char) * MD5_DIGEST_LENGTH*2 + 1)) == NULL);

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(s + (i*2), "%02X", md5[i]);

    s[MD5_DIGEST_LENGTH*2] = '\0';

    return s;
err:
    if (s)
        free(s);

    return NULL;
}
