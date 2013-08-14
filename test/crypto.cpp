#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "crypto.h"

#define TEST_CRYPTO_OUTFILE "/tmp/libece-test-key.pem"

#ifdef __cplusplus
extern "C" {
#endif

int test_crypto ()
{
    TEST_TRACE;

    if (
            ece_crypto_init(NULL)
            ||
            ece_crypto_genkey(NULL, 1024, TEST_CRYPTO_OUTFILE)
            ||
            ece_crypto_gencsr(NULL, TEST_CRYPTO_OUTFILE, NULL, NULL)
            ||
            ece_crypto_genkey(NULL, 2048, TEST_CRYPTO_OUTFILE)
            ||
            ece_crypto_gencsr(NULL, TEST_CRYPTO_OUTFILE, NULL, NULL)
            ||
            ece_crypto_genkey(NULL, 4096, TEST_CRYPTO_OUTFILE)
            ||
            ece_crypto_gencsr(NULL, TEST_CRYPTO_OUTFILE, NULL, NULL)
            ||
            ece_crypto_term(NULL)
            )
        goto err;

    return 0;

err:
    return ~0;
}

#ifdef __cplusplus
}
#endif
