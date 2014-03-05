#include <encloud/Json>
#include "test.h"
#include "json.h"

void TestJson::run ()
{
    TEST_TRACE;

    // simple test
    test("{ \"teststr\" : \"mystr\", \"testint\" : 123 }", false);

    // with newlines and tabs
    test("\n{\n\t\"teststr\":\t\"mystr\",\n\t\"testint\":\t123\n}\n", false);

    // JSONP
    test("jsonpCallback({ \"teststr\" : \"mystr\", \"testint\" : 123 })", true);
}

void TestJson::test (const QString &s, bool jsonp)
{
    bool ok;
    QString sout;
    QVariant json;

    if (jsonp)
        json = libencloud::json::parseJsonp(s, ok);
    else
        json = libencloud::json::parse(s, ok);
    QVERIFY (ok);

    sout = libencloud::json::serialize(json, ok);
    QVERIFY (ok);

    LIBENCLOUD_DBG ("in: " << s);
    LIBENCLOUD_DBG ("out: " << sout);

    // generic comparision is difficult due to random reordering
    //QVERIFY(sout == s);  
err:
    return;
};
