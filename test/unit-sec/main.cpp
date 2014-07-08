#include <QCoreApplication>
#include <QtTest>
#include "sec.h"

int main (int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TestSec secTest;
    QTest::qExec(&secTest, argc, argv);

    return EXIT_FAILURE;
}
