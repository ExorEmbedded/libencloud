#include <stdio.h>
#include <stdlib.h>
#include <encloud.h>
#include "test.h"

int main (int argc, char *argv[])
{
    encloud_rc rc = ENCLOUD_RC_SUCCESS;

    encloud_t *encloud = NULL;

    fprintf(stderr, "# testing libencloud version %s (rev: %s)\n",
            encloud_version(), encloud_revision());

    TEST_ZERO (rc = encloud_create(argc, argv, &encloud));

    TEST_ZERO (rc = encloud_retr_sb_info(encloud, NULL));
    TEST_ZERO (rc = encloud_retr_sb_cert(encloud));

    TEST_ZERO (rc = encloud_retr_sb_conf(encloud, NULL));

    encloud_destroy(encloud);

    return EXIT_SUCCESS;

err:
    if (encloud)
        encloud_destroy(encloud);

    fprintf (stderr, "# libencloud error (%d): %s\n", rc, encloud_strerror(rc));

    return EXIT_FAILURE;
}
