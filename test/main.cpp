#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <encloud/Common>
#include <common/utils.h>
#include <common/config.h>
#include "test.h"

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
    LIBENCLOUD_UNUSED(argc);
    LIBENCLOUD_UNUSED(argv);

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

    TEST_ZERO (test_json());
    TEST_ZERO (test_crypto());
#if 0
    TEST_ZERO (test_libencloud(argc, argv));
#endif

    qDebug() << "# All tests passed.";
    return EXIT_SUCCESS;

err:
    qDebug() <<"# Tests KO!";
    return EXIT_FAILURE;
}
