#include <stdio.h>
#include <stdlib.h>
#include <ece.h>

int main (int argc, char *argv[])
{
    ece_rc_t rc = ECE_RC_SUCCESS;

    ece_t *ece = NULL;

    fprintf(stderr, "# testing libece version %s\n", ece_version());

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

    fprintf (stderr, "# libece error (%d): %s\n", rc, ece_strerror(rc));

    return EXIT_FAILURE;
}
