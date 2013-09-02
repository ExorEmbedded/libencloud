#include "test.h"
#include "json.h"

#ifdef __cplusplus
extern "C" {
#endif

int test_json ()
{
    bool ok;
    QString sin, sout;
    QVariant json;
    QVariantMap jm;

    TEST_TRACE;

    // simple test
    sin = "{ \"teststr\" : \"mystr\", \"testint\" : 123 }";
    json = EceJson::parse(sin, ok);
    jm = json.toMap();

    TEST_ZERO (strcmp(qPrintable(jm["teststr"].toString()), "mystr"));
    TEST_EQUALS (jm["testint"].toInt(), 123);

    sout = EceJson::serialize(json, ok);

    ECE_DBG("sin: " << sin);
    ECE_DBG("sout: " << sout);

    // with newlines and tabs
    sin = "\n{\n\t\"teststr\":\t\"mystr\",\n\t\"testint\":\t123\n}\n";
    json = EceJson::parse(sin, ok);

    TEST_ZERO (strcmp(qPrintable(jm["teststr"].toString()), "mystr"));
    TEST_EQUALS (jm["testint"].toInt(), 123);

    sout = EceJson::serialize(json, ok);

    ECE_DBG("sin: " << sin);
    ECE_DBG("sout: " << sout);

    return 0;

err:
    return ~0;
}

#ifdef __cplusplus
}
#endif
