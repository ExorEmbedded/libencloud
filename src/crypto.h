/** 
 * Helper crypto lib.
 * 
 * Note: kept Qt-independent for possible reusage.
 */

#ifndef _ECE_CRYPTO_H_
#define _ECE_CRYPTO_H_

#include <openssl/bio.h>
#include <openssl/md5.h>
#include <ece.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Subject name setting callback - must return 0 on success */
typedef int (*ece_crypto_name_cb) (X509_NAME *n, void *arg);

struct ece_crypto_s
{
    ece_crypto_name_cb name_cb;
    void *name_cb_ctx;
};
typedef struct ece_crypto_s ece_crypto_t;

/* Initialization/Termination */
ECE_DLLSPEC int ece_crypto_init (ece_crypto_t *ec);
ECE_DLLSPEC int ece_crypto_term (ece_crypto_t *ec);

/* Callbacks */
ECE_DLLSPEC int ece_crypto_set_name_cb (ece_crypto_t *ec, int cb(X509_NAME *n, void *arg), void *ctx);

/* Crypto methods */
ECE_DLLSPEC int ece_crypto_genkey (ece_crypto_t *ec, size_t nbits, const char *outfile);
ECE_DLLSPEC int ece_crypto_gencsr (ece_crypto_t *ec, const char *keyfile, char **buf, long *buf_sz);
ECE_DLLSPEC char * ece_crypto_md5 (ece_crypto_t *ec, char *buf, long buf_sz);

#ifdef __cplusplus
}
#endif

#endif
