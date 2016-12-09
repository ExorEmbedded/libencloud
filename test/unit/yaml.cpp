#include "test.h"
#include "Yaml.h"
#include <qyamldocument.h>
#include <qyamlmapping.h>
#include <qyamlsequence.h>

void TestYaml::run ()
{
    TEST_TRACE;

#if 0
    // simple test
    test("{ \"teststr\" : \"mystr\", \"testint\" : 123 }", false);

    // with newlines and tabs
    test("\n{\n\t\"teststr\":\t\"mystr\",\n\t\"testint\":\t123\n}\n", false);

    // JSONP
    test("jsonpCallback({ \"teststr\" : \"mystr\", \"testint\" : 123 })", true);
#endif
    test();
}

void TestYaml::test ()
{
    QString s =
"foo: bar                   \n"\
"tree:                      \n"\
"   - id: 1                 \n"\
"     val: x                \n"\
"     leaf:                 \n"\
"        - id:  1           \n"\
"          val: a           \n"\
"   - id: 2                 \n"\
"     val: y                \n"\
"     leaf:                 \n"\
"        - id:  1           \n"\
"          val: b           \n"\
"        - id:  2           \n"\
"          val: c           \n"\
"        - id:  3           \n"\
"          val: d           \n";

    QtYAML::DocumentList docs = QtYAML::Document::fromYaml(s.toUtf8());
    QtYAML::Mapping mapping = docs.first().mapping();
    QtYAML::Sequence ids;

    TEST_EQUALS(mapping["foo"].toString(), "bar");

    ids = mapping["tree"].toSequence();
    TEST_EQUALS(ids.size(), 2);

    mapping = ids[1].toMapping();
    TEST_EQUALS(mapping["val"].toString(), "y");

    ids = mapping["leaf"].toSequence();
    TEST_EQUALS(ids.size(), 3);

    mapping = ids[2].toMapping();
    TEST_EQUALS(mapping["val"].toString(), "d");
err:
    return;
};
