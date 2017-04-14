#include <yaml-cpp/yaml.h>
#include "test.h"
#include "Yaml.h"

void TestYaml::run ()
{
    TEST_TRACE;

    test();
}

void TestYaml::test ()
{
    std::string input = 
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

    std::stringstream stream(input);
    YAML::Parser parser(stream);
    YAML::Node doc;
    parser.GetNextDocument(doc);

    const YAML::Node &doc2 = doc["tree"];
    const YAML::Node &doc3 = doc2[1]["leaf"];
    std::string s;

    doc["foo"] >> s;
    TEST_EQUALS(QString::fromUtf8(s.c_str()), "bar");
    TEST_EQUALS(doc2.size(), 2);

    doc2[0]["val"] >> s;
    TEST_EQUALS(QString::fromUtf8(s.c_str()), "x");

    doc2[1]["val"] >> s;
    TEST_EQUALS(QString::fromUtf8(s.c_str()), "y");

    TEST_EQUALS(doc3.size(), 3);

    doc3[2]["val"] >> s;
    TEST_EQUALS(QString::fromUtf8(s.c_str()), "d");
    
err:
    return;
};
