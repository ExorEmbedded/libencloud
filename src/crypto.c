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
#undef __ECE_MSG
#define __ECE_MSG(lev,msg) __ECE_PRINT(lev, msg)
#include "crypto.h"

static X509_REQ *__make_req (ece_crypto_t *ec, EVP_PKEY *pkey);

int ece_crypto_init (ece_crypto_t *ec)
{
    ECE_TRACE;

    if (ec == NULL)  // context is optional for now
        return 0;

    memset(ec, 0, sizeof(ece_crypto_t));

    OpenSSL_add_all_algorithms();

    return 0;
}

int ece_crypto_term (ece_crypto_t *ec)
{
    ECE_TRACE;
    ECE_UNUSED(ec);

    return 0;
}

int ece_crypto_set_name_cb (ece_crypto_t *ec, int cb(X509_NAME *n, void *arg), void *ctx)
{
    ECE_ERR_IF (ec == NULL);
    ECE_ERR_IF (cb == NULL);

    ec->name_cb = cb;
    ec->name_cb_ctx = ctx;

    return 0;
err:
    return ~0;
}

/** \brief Generate an RSA key of size 'nbits' to 'outfile' */
int ece_crypto_genkey (ece_crypto_t *ec, size_t nbits, const char *outfile)
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

    ECE_TRACE;
    ECE_UNUSED(ec);
    ECE_ERR_IF (outfile == NULL);

    // make sure random number generator is adequately seeded
    ECE_ERR_IF ((file = RAND_file_name(rbuf, sizeof(rbuf))) == NULL);
    ECE_ERR_IF (!RAND_load_file(file, -1));
    ECE_ERR_IF (!RAND_status());

    // create output file
    ECE_ERR_IF ((out = BIO_new(BIO_s_file())) == NULL);
    ECE_ERR_IF (BIO_write_filename(out, (char *) outfile) <= 0);

    // run RSA keygen algo
    ECE_ERR_IF ((bn = BN_new()) == NULL);
    ECE_ERR_IF ((rsa = RSA_new()) == NULL);
    ECE_ERR_IF (!BN_set_word(bn, f4));
    ECE_ERR_IF (!RSA_generate_key_ex(rsa, nbits, bn, NULL));
    ECE_ERR_IF (!PEM_write_bio_RSAPrivateKey(out, rsa, enc, NULL, 0, NULL, NULL));

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
 *  Prerequisites: ece_crypto_set_name_cb() for subject name settings
 */
int ece_crypto_gencsr (ece_crypto_t *ec, const char *keyfile, char **pbuf, long *plen)
{
    int rc = ~0;
    BIO *kfile = NULL;
    BIO *out = NULL;
    EVP_PKEY *pkey = NULL;
    X509_REQ *req = NULL;
    long errline = -1;
    char *buf = NULL, *pb;
    long len = 0;

    ECE_TRACE;
    ECE_ERR_IF (keyfile == NULL);
    ECE_ERR_IF (ec == NULL);
    ECE_ERR_IF (ec->name_cb == NULL);

    ECE_ERR_IF ((kfile = BIO_new(BIO_s_file())) == NULL);
    ECE_ERR_IF (BIO_read_filename(kfile, keyfile) <= 0);

    // load private key
    ECE_ERR_IF ((pkey = PEM_read_bio_PrivateKey(kfile, NULL, NULL, NULL)) == NULL);

    // generate the CSR
    ECE_ERR_IF ((req = __make_req(ec, pkey)) == NULL);

    if (pbuf == NULL)  // output to standard output
    {
        ECE_ERR_IF ((out = BIO_new(BIO_s_file())) == NULL);
        BIO_set_fp(out, stdout, BIO_NOCLOSE);

        ECE_ERR_IF (!PEM_write_bio_X509_REQ(out, req));
    }
    else 
    {
        ECE_ERR_IF ((out = BIO_new(BIO_s_mem())) == NULL);

        ECE_ERR_IF (!PEM_write_bio_X509_REQ(out, req));
        BIO_flush(out);

        // memory to buffer
        ECE_ERR_IF ((len = BIO_get_mem_data(out, &pb)) <= 0);
        ECE_ERR_IF ((buf = calloc(1, sizeof(char) * len)) == NULL);
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

static X509_REQ *__make_req (ece_crypto_t *ec, EVP_PKEY *pkey)
{
    X509_REQ *req = NULL;
    X509_NAME *n = NULL;
    EVP_MD_CTX mctx;
    EVP_MD_CTX_init(&mctx);
    EVP_PKEY_CTX *pkctx = NULL;
    const EVP_MD *digest = NULL;

    ECE_ERR_IF (ec == NULL);
    ECE_ERR_IF (pkey == NULL);
    ECE_ERR_IF (ec->name_cb == NULL);

    ECE_ERR_IF ((req = X509_REQ_new()) == NULL);
    ECE_ERR_IF ((n = X509_NAME_new()) == NULL);
    
    ECE_ERR_IF (ec->name_cb(n, ec->name_cb_ctx));
    ECE_ERR_IF (!X509_REQ_set_subject_name(req, n));
    ECE_ERR_IF (!X509_REQ_set_pubkey(req, pkey));

    ECE_ERR_IF ((digest = EVP_get_digestbyname(SN_sha256)) == NULL);
    ECE_ERR_IF (!EVP_DigestSignInit(&mctx, &pkctx, digest, NULL, pkey));
    ECE_ERR_IF (!X509_REQ_sign_ctx(req, &mctx));
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
