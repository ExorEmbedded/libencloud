/** 
 * Helper crypto lib.
 * 
 * Note: kept Qt-independent for possible reusage.
 */

#ifndef _ENCLOUD_PRIV_CRYPTO_H_
#define _ENCLOUD_PRIV_CRYPTO_H_

#include <encloud/common.h>
#include <openssl/bio.h>
#include <openssl/md5.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Subject name setting callback - must return 0 on success */
typedef int (*encloud_crypto_name_cb) (X509_NAME *n, void *arg);

struct encloud_crypto_s
{
    encloud_crypto_name_cb name_cb;
    void *name_cb_ctx;
};
typedef struct encloud_crypto_s encloud_crypto_t;

/* Initialization/Termination */
ENCLOUD_DLLSPEC int encloud_crypto_init (encloud_crypto_t *ec);
ENCLOUD_DLLSPEC int encloud_crypto_term (encloud_crypto_t *ec);

/* Callbacks */
ENCLOUD_DLLSPEC int encloud_crypto_set_name_cb (encloud_crypto_t *ec, int cb(X509_NAME *n, void *arg), void *ctx);

/* Crypto methods */
ENCLOUD_DLLSPEC int encloud_crypto_genkey (encloud_crypto_t *ec, size_t nbits, const char *outfile);
ENCLOUD_DLLSPEC int encloud_crypto_gencsr (encloud_crypto_t *ec, const char *keyfile, char **buf, long *buf_sz);
ENCLOUD_DLLSPEC char * encloud_crypto_md5 (encloud_crypto_t *ec, char *buf, long buf_sz);

#ifdef __cplusplus
}
#endif

#endif  /* _ENCLOUD_PRIV_CRYPTO_H_ */
