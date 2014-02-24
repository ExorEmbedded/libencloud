/** 
 * Helper crypto lib.
 */

#ifndef _LIBENCLOUD_PRIV_CRYPTO_H_
#define _LIBENCLOUD_PRIV_CRYPTO_H_

#include <encloud/Common>
#include <openssl/bio.h>
#include <openssl/md5.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Subject name setting callback - must return 0 on success */
typedef int (*libencloud_crypto_name_cb) (X509_NAME *n, void *arg);

struct libencloud_crypto_s
{
    libencloud_crypto_name_cb name_cb;
    void *name_cb_ctx;
};
typedef struct libencloud_crypto_s libencloud_crypto_t;

/* Initialization/Termination */
LIBENCLOUD_DLLSPEC int libencloud_crypto_init (libencloud_crypto_t *ec);
LIBENCLOUD_DLLSPEC int libencloud_crypto_term (libencloud_crypto_t *ec);

/* Callbacks */
LIBENCLOUD_DLLSPEC int libencloud_crypto_set_name_cb (libencloud_crypto_t *ec, int cb(X509_NAME *n, void *arg), void *ctx);

/* Crypto methods */
LIBENCLOUD_DLLSPEC int libencloud_crypto_genkey (libencloud_crypto_t *ec, size_t nbits, const char *outfile);
LIBENCLOUD_DLLSPEC int libencloud_crypto_gencsr (libencloud_crypto_t *ec, const char *keyfile, char **buf, long *buf_sz);
LIBENCLOUD_DLLSPEC char * libencloud_crypto_md5 (libencloud_crypto_t *ec, char *buf, long buf_sz);

#ifdef __cplusplus
}
#endif

#endif  /* _LIBENCLOUD_PRIV_CRYPTO_H_ */
