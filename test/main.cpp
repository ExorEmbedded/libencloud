#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include "ece.h"
#include "utils.h"
#include "test.h"

#ifdef __cplusplus
extern "C" {
#endif

int test_json ();
int test_crypto ();
int test_ece (int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

int main (int argc, char *argv[])
{
    qDebug() << "#";
    qDebug() << "# testing libece version " << ece_version() << " (commit: " << ece_revision() <<  ")";
    qDebug() << "#";

#ifdef Q_OS_UNIX
    qDebug() << "# OS: Unix";
#endif
#ifdef Q_OS_WIN32
    qDebug() << "# OS: W32";
#endif
    qDebug() << "# hw_info: " << EceUtils::getHwInfo();

    TEST_ZERO (test_json());
    TEST_ZERO (test_crypto());
    TEST_ZERO (test_ece(argc, argv));

    qDebug() << "# All tests passed.";
    return EXIT_SUCCESS;

err:
    qDebug() <<"# Tests KO!";
    return EXIT_FAILURE;
}
