#ifndef _ECE_CRYPTO_H_
#define _ECE_CRYPTO_H_

#include <openssl/bio.h>
#include <ece.h>

#ifdef __cplusplus
extern "C" {
#endif

int ece_crypto_init (ece_t *ece);
int ece_crypto_term (ece_t *ece);

int ece_crypto_genkey (ece_t *ece, size_t nbits, const char *outfile);
int ece_crypto_gencsr (ece_t *ece, const char *keyfile, char **buf, long *buf_sz);

#ifdef __cplusplus
}
#endif

#endif
