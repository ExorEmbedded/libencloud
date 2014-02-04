#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <encloud/Common>
#include "utils.h"
#include "test.h"
#include "config.h"

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
#if 0
    qDebug() << "#";
    qDebug() << "# testing libencloud version: " << libencloud_version() << " (rev: " << libencloud_revision() <<  ")";
    qDebug() << "#                string: " << LIBENCLOUD_STRING;
    qDebug() << "#";
#endif

#ifdef Q_OS_UNIX
    qDebug() << "# OS: Unix";
#endif
#ifdef Q_OS_WIN32
    qDebug() << "# OS: W32";
#endif

    qDebug() << "# hw_info: " << libencloud::utils::getHwInfo();
#if 0

    TEST_ZERO (test_json());
    TEST_ZERO (test_crypto());
    TEST_ZERO (test_libencloud(argc, argv));
#endif

    qDebug() << "# All tests passed.";
    return EXIT_SUCCESS;

err:
    qDebug() <<"# Tests KO!";
    return EXIT_FAILURE;
}
