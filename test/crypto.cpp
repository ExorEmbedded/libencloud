#include <openssl/x509v3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "crypto.h"

#define TEST_CRYPTO_OUTFILE "/tmp/libece-test-key.pem"

#ifdef __cplusplus
extern "C" {
#endif

static int __name_cb (X509_NAME *n, void *arg)
{
    ECE_UNUSED(arg);

    X509_NAME_add_entry_by_txt(n, "C", MBSTRING_ASC, (const unsigned char *) "AU", -1, -1, 0);

    return 0;
}

int test_crypto ()
{
    char *s = NULL;
    TEST_TRACE;

    ece_crypto_t ec;

    TEST_ZERO (ece_crypto_init(&ec));

    TEST_ZERO (ece_crypto_set_name_cb(&ec, &__name_cb, NULL));

    TEST_ZERO (ece_crypto_genkey(&ec, 1024, TEST_CRYPTO_OUTFILE));
    TEST_ZERO (ece_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL));

    TEST_ZERO (ece_crypto_genkey(&ec, 2048, TEST_CRYPTO_OUTFILE));
    TEST_ZERO (ece_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL));

    TEST_ZERO (ece_crypto_genkey(&ec, 4096, TEST_CRYPTO_OUTFILE));
    TEST_ZERO (ece_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL));

    TEST_ZERO (ece_crypto_term(&ec));

    s = ece_crypto_md5(&ec, (char *) "foo bar", strlen("foo bar"));
    TEST_ZERO (strcmp(s, "327B6F07435811239BC47E1544353273"));

    free(s);

    return 0;

err:
    if (s)
        free(s);
    return ~0;
}

#ifdef __cplusplus
}
#endif
