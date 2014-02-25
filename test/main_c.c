#include <stdio.h>
#include <stdlib.h>
common.h>.h>
#include "test.h"

int main (int argc, char *argv[])
{
    libencloud_rc rc = LIBENCLOUD_RC_SUCCESS;

    libencloud_t *libencloud = NULL;

    fprintf(stderr, "# testing libencloud version %s (rev: %s)\n",
            libencloud_version(), libencloud_revision());

    TEST_ZERO (rc = libencloud_create(argc, argv, &libencloud));

    TEST_ZERO (rc = libencloud_retr_sb_info(libencloud, NULL));
    TEST_ZERO (rc = libencloud_retr_sb_cert(libencloud));

    TEST_ZERO (rc = libencloud_retr_sb_conf(libencloud, NULL));

    libencloud_destroy(libencloud);

    return EXIT_SUCCESS;

err:
    if (libencloud)
        libencloud_destroy(libencloud);

    fprintf (stderr, "# libencloud error (%d): %s\n", rc, libencloud_strerror(rc));

    return EXIT_FAILURE;
}
