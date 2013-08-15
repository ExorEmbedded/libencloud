#include <openssl/x509v3.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "crypto.h"

#define TEST_CRYPTO_OUTFILE "/tmp/libece-test-key.pem"

#ifdef __cplusplus
extern "C" {
#endif

static int __name_cb (X509_NAME *n, void *arg)
{
    X509_NAME_add_entry_by_txt(n, "C", MBSTRING_ASC, (const unsigned char *) "AU", -1, -1, 0);

    return 0;
}

int test_crypto ()
{
    TEST_TRACE;

    ece_crypto_t ec;

    if (
            ece_crypto_init(&ec)
            ||
            ece_crypto_set_name_cb(&ec, &__name_cb, NULL)
            ||
            ece_crypto_genkey(&ec, 1024, TEST_CRYPTO_OUTFILE)
            ||
            ece_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL)
            ||
            ece_crypto_genkey(&ec, 2048, TEST_CRYPTO_OUTFILE)
            ||
            ece_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL)
            ||
            ece_crypto_genkey(&ec, 4096, TEST_CRYPTO_OUTFILE)
            ||
            ece_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL)
            ||
            ece_crypto_term(&ec)
            )
        goto err;

    return 0;

err:
    return ~0;
}

#ifdef __cplusplus
}
#endif
