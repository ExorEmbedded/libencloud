//#define QTCORE_APP_EXTERNAL
#ifdef QTCORE_APP_EXTERNAL
#include <QtCore/QCoreApplication>
#endif
#include <QUuid>
#include <stdio.h>
#include <stdlib.h>
#include <ece.h>
#include "test.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Default is a "Pure C" libece test (no Qt objects): an internal
 * QCoreApplication is created if no instance is found!
 *
 * To run an external application define QTCORE_APP_EXTERNAL.
 */
int test_ece (int argc, char *argv[])
{
    TEST_TRACE;

    ece_rc_t rc = ECE_RC_SUCCESS;
    ece_t *ece = NULL;
    ece_sb_info_t *sb_info;
    ece_sb_conf_t *sb_conf;

#ifdef QTCORE_APP_EXTERNAL
    QCoreApplication a(argc, argv);
#endif

    TEST_ZERO ((rc = ece_create(argc, argv, &ece)));

#ifdef ECE_TYPE_SECE
    fprintf(stderr, "# type: SECE\n");

    // bad value - must fail
    TEST_ZERO ((rc = ece_set_license(ece, "a-b-c-d")) == 0);

    // generate a random uuid
    TEST_ZERO ((rc = ece_set_license(ece, qPrintable(QUuid::createUuid().toString()))));

    // variant() = 2 (Distributed Computing Environment), version() = 4 (Random-based)
    TEST_ZERO ((rc = ece_set_license(ece, "67C8770B-44F1-410A-AB9A-F9B5446F13EE")));

    // variant() = 2 (Distributed Computing Environment), version() = 1 (Time-based)
    TEST_ZERO ((rc = ece_set_license(ece, "{a8098c1a-f86e-11da-bd1a-00112444be1e}")));

#else
    fprintf(stderr, "# type: ECE\n");
    fprintf(stderr, "# serial: %s\n", ece_get_serial(ece));
    fprintf(stderr, "# poi: %s\n", ece_get_poi(ece));
#endif

    TEST_ZERO ((rc = ece_retr_sb_info(ece, &sb_info)));
    TEST_ZERO ((rc = ece_retr_sb_cert(ece)));
    TEST_ZERO ((rc = ece_retr_sb_conf(ece, &sb_conf)));

    fprintf(stderr, "# license valid: %d\n", ece_sb_info_get_license_valid(sb_info));
    fprintf(stderr, "# license expiry: %ld\n", ece_sb_info_get_license_expiry(sb_info));
    fprintf(stderr, "# vpn ip: %s\n", ece_sb_conf_get_vpn_ip(sb_conf));
    fprintf(stderr, "# vpn port: %d\n", ece_sb_conf_get_vpn_port(sb_conf));
    fprintf(stderr, "# vpn proto: %s\n", ece_sb_conf_get_vpn_proto(sb_conf));
    fprintf(stderr, "# vpn type: %s\n", ece_sb_conf_get_vpn_type(sb_conf));

    ece_destroy(ece);

    return 0;

err:
    if (ece)
        ece_destroy(ece);

    fprintf (stderr, "# libece error (%d): %s\n", rc, ece_strerror(rc));

    return ~0;
}

#ifdef __cplusplus
}
#endif
