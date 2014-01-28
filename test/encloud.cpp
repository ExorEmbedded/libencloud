//#define QTCORE_APP_EXTERNAL
#ifdef QTCORE_APP_EXTERNAL
#include <QtCore/QCoreApplication>
#endif
#include <QUuid>
#include <stdio.h>
#include <stdlib.h>
#include <encloud.h>
#include "test.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_TOUT_MINS 5

/**
 * Default is a "Pure C" libencloud test (no Qt objects): an internal
 * QCoreApplication is created if no instance is found!
 *
 * To run an external application define QTCORE_APP_EXTERNAL.
 */
int test_encloud (int argc, char *argv[])
{
    TEST_TRACE;

    encloud_rc_t rc = ENCLOUD_RC_SUCCESS;
    encloud_t *encloud = NULL;
    encloud_sb_info_t *sb_info;
    encloud_sb_conf_t *sb_conf;

#ifdef QTCORE_APP_EXTERNAL
    QCoreApplication a(argc, argv);
#endif

    TEST_ZERO ((rc = encloud_create(argc, argv, &encloud)));

#ifdef ENCLOUD_TYPE_SECE
    fprintf(stderr, "# type: SECE\n");

    // bad value - must fail
    TEST_ZERO ((rc = encloud_set_license(encloud, "a-b-c-d")) == 0);

    // generate a random uuid
    TEST_ZERO ((rc = encloud_set_license(encloud, qPrintable(QUuid::createUuid().toString()))));

    // variant() = 2 (Distributed Computing Environment), version() = 4 (Random-based)
    TEST_ZERO ((rc = encloud_set_license(encloud, "67C8770B-44F1-410A-AB9A-F9B5446F13EE")));

    // variant() = 2 (Distributed Computing Environment), version() = 1 (Time-based)
    TEST_ZERO ((rc = encloud_set_license(encloud, "{a8098c1a-f86e-11da-bd1a-00112444be1e}")));

#else
    fprintf(stderr, "# type: ECE\n");
    fprintf(stderr, "# serial: %s\n", encloud_get_serial(encloud));
    fprintf(stderr, "# poi: %s\n", encloud_get_poi(encloud));
#endif

    TEST_ZERO_RETRY ((rc = encloud_retr_sb_info(encloud, &sb_info)), TEST_TOUT_MINS);
    TEST_ZERO_RETRY ((rc = encloud_retr_sb_cert(encloud)), TEST_TOUT_MINS);
    TEST_ZERO_RETRY ((rc = encloud_retr_sb_conf(encloud, &sb_conf)), TEST_TOUT_MINS);

    fprintf(stderr, "# license valid: %d\n", encloud_sb_info_get_license_valid(sb_info));
    fprintf(stderr, "# license expiry: %ld\n", encloud_sb_info_get_license_expiry(sb_info));
    fprintf(stderr, "# vpn ip: %s\n", encloud_sb_conf_get_vpn_ip(sb_conf));
    fprintf(stderr, "# vpn port: %d\n", encloud_sb_conf_get_vpn_port(sb_conf));
    fprintf(stderr, "# vpn proto: %s\n", encloud_sb_conf_get_vpn_proto(sb_conf));
    fprintf(stderr, "# vpn type: %s\n", encloud_sb_conf_get_vpn_type(sb_conf));

    encloud_destroy(encloud);

    return 0;

err:
    if (encloud)
        encloud_destroy(encloud);

    fprintf (stderr, "# libencloud error (%d): %s\n", rc, encloud_strerror(rc));

    return ~0;
}

#ifdef __cplusplus
}
#endif
