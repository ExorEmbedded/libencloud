#include <encloud/Json>
#include <encloud/Utils>
#include "test.h"
#include "utils.h"

void TestUtils::run ()
{
    TEST_TRACE;

    testMapMerge(
            "{ \"foo\" : 123 }",
            "{ \"foo\" : 456 }",
            "{ \"foo\" : 456 }"
            );

    testMapMerge(
            "{}",
            "{ \"foo\" : 123 }",
            "{ \"foo\" : 123 }"
            );

    testMapMerge(
            "{ \"foo\" : 123 }",
            "{ \"bar\" : 456 }",
            "{ \"foo\" : 123, \"bar\" : 456 }"
            );

    testMapMerge(
            "{ \"foo\" : 123 }",
            "{ \"bar\" : 456, "
                "\"sub\" : { \"a\" : 1, \"b\" : 2 }"
            "}",
            "{ \"foo\" : 123, \"bar\" : 456, "
                "\"sub\" : { \"a\" : 1, \"b\" : 2 }"
            "}"
            );

    testMapMerge(
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

    testMapMerge(
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

    testMapMerge(
            "{ \"a\" : 1, \"b\" : 2 }",
            "{ \"a\" : { \"a1\" : 11, \"a2\" : 12 }, \"c\" : 3 }",
            "{ \"a\" : { \"a1\" : 11, \"a2\" : 12 }, \"b\" : 2, \"c\" : 3 }"
            );

    testMapMerge(
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

void TestUtils::testMapMerge (const QString &to, const QString &from, const QString &exp)
{
    bool ok = true;
    QVariantMap mfrom;
    QVariantMap mto;
    QVariantMap mexp;

    LIBENCLOUD_DBG ("IN to: " << to << ", from: " << from << ", exp: " << exp);

    mfrom = libencloud::json::parse(from, ok).toMap();
    QVERIFY (ok);

    mto = libencloud::json::parse(to, ok).toMap();
    QVERIFY (ok);

    mexp = libencloud::json::parse(exp, ok).toMap();
    QVERIFY (ok);

    libencloud::utils::mapMerge(mto, mfrom);

    LIBENCLOUD_DBG ("OUT to: " << libencloud::json::serialize(mto, ok));

    QVERIFY(mto == mexp);
};
