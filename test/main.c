//#define QTCORE_APP_EXTERNAL
#ifdef QTCORE_APP_EXTERNAL
#include <QtCore/QCoreApplication>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ece.h>

/**
 * Default is a "Pure C" libece test (no Qt objects): an internal
 * QCoreApplication is created if no instance is found!
 *
 * To run an external application define QTCORE_APP_EXTERNAL.
 */
int main (int argc, char *argv[])
{
    ece_rc_t rc = ECE_RC_SUCCESS;

#ifdef QTCORE_APP_EXTERNAL
    QCoreApplication a(argc, argv);
#endif

    ece_t *ece = NULL;

    fprintf(stderr, "testing libece version %s\n", ece_version());

    if (
            (rc = ece_create(argc, argv, &ece))
            ||
            (rc = ece_retr_sb_info(ece))
            ||
            (rc = ece_retr_sb_cert(ece))
            ||
            (rc = ece_retr_sb_conf(ece))
       )
        goto err;

    ece_destroy(ece);

    return EXIT_SUCCESS;

err:
    if (ece)
        ece_destroy(ece);

    fprintf (stderr, "libece error (%d): %s\n", rc, ece_strerror(rc));

    return EXIT_FAILURE;
}
