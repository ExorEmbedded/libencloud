#include <encloud/Crypto>
#include <openssl/x509v3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"

#ifdef Q_OS_WIN
#define TEST_CRYPTO_OUTFILE "c:\\temp\\libencloud-test-key.pem"
#else
#define TEST_CRYPTO_OUTFILE "/tmp/libencloud-test-key.pem"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int test_crypto_encdec (libencloud_crypto_t *ec, const char *key, const char *ptext);

static int __name_cb (X509_NAME *n, void *arg)
{
    LIBENCLOUD_UNUSED(arg);

    X509_NAME_add_entry_by_txt(n, "C", MBSTRING_ASC, (const unsigned char *) "AU", -1, -1, 0);

    return 0;
}

int test_crypto ()
{
    char *s = NULL;
    TEST_TRACE;

    libencloud_crypto_t ec;

    unsigned char md[LIBENCLOUD_CRYPTO_MAX_MD_SZ];
    unsigned int md_sz;

    TEST_ZERO (libencloud_crypto_init(&ec));
    TEST_ZERO (libencloud_crypto_set_zeropad(&ec, true));
    TEST_ZERO (libencloud_crypto_set_name_cb(&ec, &__name_cb, NULL));

    TEST_ZERO (libencloud_crypto_genkey(&ec, 1024, TEST_CRYPTO_OUTFILE));
    TEST_ZERO (libencloud_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL));

    TEST_ZERO (libencloud_crypto_genkey(&ec, 2048, TEST_CRYPTO_OUTFILE));
    TEST_ZERO (libencloud_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL));

    TEST_ZERO (libencloud_crypto_genkey(&ec, 4096, TEST_CRYPTO_OUTFILE));
    TEST_ZERO (libencloud_crypto_gencsr(&ec, TEST_CRYPTO_OUTFILE, NULL, NULL));

    s = libencloud_crypto_md5_hex(&ec, (unsigned char *) "foo bar", strlen("foo bar"));
    TEST_ZERO (strcmp(s, "327B6F07435811239BC47E1544353273"));

    TEST_ZERO (libencloud_crypto_digest(&ec, (unsigned char *) "foo bar", strlen("foo bar"), md, &md_sz));
    TEST_EQUALS (QByteArray((const char *) md, md_sz).toHex().toUpper(), "327B6F07435811239BC47E1544353273");

    TEST_ZERO (test_crypto_encdec(&ec, "123", "x"));
    TEST_ZERO (test_crypto_encdec(&ec, "01234567890123456789012345678901", "x               ")); //=block size
    TEST_ZERO (test_crypto_encdec(&ec, "01234567890123456789012345678901", "my secret text_"));
    TEST_ZERO (test_crypto_encdec(&ec, "01234567890123456789012345678901", "my secret text__")); //=block size
    TEST_ZERO (test_crypto_encdec(&ec, "01234567890123456789012345678901", "my secret text__my secret text__"));
    TEST_ZERO (test_crypto_encdec(&ec, "01234567890123456789012345678901", "Some little secret nobody should know about..."));
    TEST_ZERO (test_crypto_encdec(&ec, "01234567890123456789012345678901", "my secret text__"));
    TEST_ZERO (test_crypto_encdec(&ec, "01234567890123456789012345678901", "Some little secret nobody should know about... n1\n"\
                "Some little secret nobody should know about... n2\n"\
                "Some little secret nobody should know about... n3\n"\
                "Some little secret nobody should know about... n4\n"\
                "Some little secret nobody should know about... n5\n"\
                "Some little secret nobody should know about... n6\n"\
                "Some little secret nobody should know about... n8\n"\
                "Some little secret nobody should know about... n9\n"\
                "Some little secret nobody should know about... n10!!!!\n"));

    TEST_ZERO (libencloud_crypto_term(&ec));

    free(s);

    return 0;

err:
    if (s)
        free(s);
    return ~0;
}

// We assume test input is always a null-terminated string
int test_crypto_encdec (libencloud_crypto_t *ec, const char *key, const char *ptext)
{
    enum { TEXT_MAX = 1024 };
    unsigned char ctext[TEXT_MAX];
    long ctext_sz;
    unsigned char dtext[TEXT_MAX];
    long dtext_sz;
    unsigned char iv[16];

    TEST_MSG(ptext);

    memset(iv, 0, sizeof(iv));

    TEST_ZERO (libencloud_crypto_enc(ec, (unsigned char *) ptext, strlen((const char *) ptext),
            (unsigned char *)key, iv, ctext, &ctext_sz));
    ctext[ctext_sz] = '\0';

    TEST_ZERO (libencloud_crypto_dec(ec, ctext, ctext_sz,
            (unsigned char *)key, iv, dtext, &dtext_sz));
    dtext[dtext_sz] = '\0';

    TEST_ZERO (strcmp((char *) ptext, (char *) dtext));

    return 0;
err:
    return ~0;
}

#ifdef __cplusplus
}
#endif
