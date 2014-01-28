#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <encloud/core.h>
#include "utils.h"
#include "test.h"

#ifdef __cplusplus
extern "C" {
#endif

int test_json ();
int test_crypto ();
int test_encloud (int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

int main (int argc, char *argv[])
{
    qDebug() << "#";
    qDebug() << "# testing libencloud version: " << encloud_version() << " (rev: " << encloud_revision() <<  ")";
    qDebug() << "#                string: " << ENCLOUD_STRING;
    qDebug() << "#";

#ifdef Q_OS_UNIX
    qDebug() << "# OS: Unix";
#endif
#ifdef Q_OS_WIN32
    qDebug() << "# OS: W32";
#endif
    qDebug() << "# hw_info: " << encloud::utils::getHwInfo();

    TEST_ZERO (test_json());
    TEST_ZERO (test_crypto());
    TEST_ZERO (test_encloud(argc, argv));

    qDebug() << "# All tests passed.";
    return EXIT_SUCCESS;

err:
    qDebug() <<"# Tests KO!";
    return EXIT_FAILURE;
}
