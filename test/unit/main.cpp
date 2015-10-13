#include <QCoreApplication>
#include <QtTest>
#include <encloud/Common>
#include <encloud/Utils>
#include <common/config.h>
#include "test.h"
#include "utils.h"
#include "proxy.h"
#include "api.h"
#include "json.h"

#ifdef __cplusplus
extern "C" {
#endif

int test_json ();
int test_crypto ();
int test_libencloud (int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

int main (int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

#ifdef Q_OS_UNIX
    qDebug() << "# OS: Unix";
#endif
#ifdef Q_OS_WIN32
    qDebug() << "# OS: W32";
#endif
#ifdef Q_OS_WINCE
    qDebug() << "# OS: WCE";
#endif

    qDebug() << "# hw_info: " << libencloud::utils::getHwInfo();

    TestUtils utilsTest;
    QTest::qExec(&utilsTest, argc, argv);

    TestProxy proxyTest;
    QTest::qExec(&proxyTest, argc, argv);

    TestApi apiTest;
    QTest::qExec(&apiTest, argc, argv);

    TestJson jsonTest;
    QTest::qExec(&jsonTest, argc, argv);

    TEST_ZERO (test_crypto());

    qDebug() << "# All tests passed.";
    return EXIT_SUCCESS;

err:
    qDebug() <<"# Tests KO!";
    return EXIT_FAILURE;
}
