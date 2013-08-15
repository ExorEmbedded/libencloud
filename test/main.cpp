#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include "ece.h"
#include "utils.h"
#include "test.h"

#ifdef __cplusplus
extern "C" {
#endif

int test_crypto ();
int test_ece (int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

int main (int argc, char *argv[])
{
    fprintf(stderr, "#\n");
    fprintf(stderr, "# testing libece version %s\n", ece_version());
    fprintf(stderr, "#\n");

#ifdef Q_OS_UNIX
    fprintf(stderr, "# OS: Unix\n");
#endif
#ifdef Q_OS_WIN32
    fprintf(stderr, "# OS: W32\n");
#endif
    fprintf(stderr, "# hw_info: %s\n", qPrintable(EceUtils::getHwInfo()));

    TEST_ZERO (test_crypto());
    TEST_ZERO (test_ece(argc, argv));

    fprintf (stderr, "# All tests passed.\n");
    return EXIT_SUCCESS;

err:
    fprintf (stderr, "# Tests KO!\n");
    return EXIT_FAILURE;
}
