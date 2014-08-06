#include <encloud/Json>
#include <encloud/Utils>
#include "test.h"
#include "utils.h"

void TestUtils::run ()
{
    TEST_TRACE;

    testVariantMerge(
            "{ \"foo\" : 123 }",
            "{ \"foo\" : 456 }",
            "{ \"foo\" : 456 }"
            );

    testVariantMerge(
            "{}",
            "{ \"foo\" : 123 }",
            "{ \"foo\" : 123 }"
            );

    testVariantMerge(
            "{ \"foo\" : 123 }",
            "{ \"bar\" : 456 }",
            "{ \"foo\" : 123, \"bar\" : 456 }"
            );

    testVariantMerge(
            "{ \"foo\" : 123 }",
            "{ \"bar\" : 456, "
                "\"sub\" : { \"a\" : 1, \"b\" : 2 }"
            "}",
            "{ \"foo\" : 123, \"bar\" : 456, "
                "\"sub\" : { \"a\" : 1, \"b\" : 2 }"
            "}"
            );

    testVariantMerge(
            "{"
                "\"sub\" : { \"a\" : 1, \"b\" : 2 }"
            "}",
            "{"
                "\"sub\" : { \"c\" : 3 }"
            "}",
            "{"
                "\"sub\" : { \"a\" : 1, \"b\" : 2, \"c\" : 3 }"
            "}"
            );

    testVariantMerge(
            "{"
                "\"sub\" : { \"a\" : 1, \"b\" : 2 }"
            "}",
            "{"
                "\"sub\" : { \"b\" : 20, \"c\" : 30 }"
            "}",
            "{"
                "\"sub\" : { \"a\" : 1, \"b\" : 20, \"c\" : 30 }"
            "}"
            );

    testVariantMerge(
            "{ \"a\" : 1, \"b\" : 2 }",
            "{ \"a\" : { \"a1\" : 11, \"a2\" : 12 }, \"c\" : 3 }",
            "{ \"a\" : { \"a1\" : 11, \"a2\" : 12 }, \"b\" : 2, \"c\" : 3 }"
            );

    testVariantMerge(
            "{"
            "   \"log\" : false, \"ssl\" : { \"init\" : { \"verify_ca\" : false } }"
            "}",
            "{"
            "   \"log\" : true, \"ssl\" : { \"init\" : { \"ca\" : \"/tmp/ca.pem\" } }"
            "}",
            "{"
            "   \"log\" : true, \"ssl\" : { \"init\" : { \"verify_ca\" : false, \"ca\" : \"/tmp/ca.pem\" } }"
            "}"
            );
}

void TestUtils::testVariantMerge (const QString &to, const QString &from, const QString &exp)
{
    bool ok = true;
    QVariant jfrom;
    QVariant jto;
    QVariant jout;
    QVariant jexp;

    LIBENCLOUD_DBG ("IN to: " << to << ", from: " << from << ", exp: " << exp);

    jfrom = libencloud::json::parse(from, ok);
    QVERIFY (ok);

    jto = libencloud::json::parse(to, ok);
    QVERIFY (ok);

    jexp = libencloud::json::parse(exp, ok);
    QVERIFY (ok);

    libencloud::utils::variantMerge(jto, jfrom);

    LIBENCLOUD_DBG ("OUT to: " << libencloud::json::serialize(jto, ok));

    QVERIFY(jto == jexp);
};
