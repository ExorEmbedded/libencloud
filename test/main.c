#include <stdio.h>
#include <stdlib.h>
#include <ece.h>
#include "test.h"

int main (int argc, char *argv[])
{
    ece_rc_t rc = ECE_RC_SUCCESS;

    ece_t *ece = NULL;

    fprintf(stderr, "# testing libece version %s (rev: %s)\n",
            ece_version(), ece_revision());

    TEST_ZERO (rc = ece_create(argc, argv, &ece));

    TEST_ZERO (rc = ece_retr_sb_info(ece, NULL));
    TEST_ZERO (rc = ece_retr_sb_cert(ece));

    TEST_ZERO (rc = ece_retr_sb_conf(ece, NULL));

    ece_destroy(ece);

    return EXIT_SUCCESS;

err:
    if (ece)
        ece_destroy(ece);

    fprintf (stderr, "# libece error (%d): %s\n", rc, ece_strerror(rc));

    return EXIT_FAILURE;
}
