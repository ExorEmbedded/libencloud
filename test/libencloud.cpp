//#define QTCORE_APP_EXTERNAL
#ifdef QTCORE_APP_EXTERNAL
#include <QtCore/QCoreApplication>
#endif
#include <QUuid>
#include <stdio.h>
#include <stdlib.h>
#include <encloud/core.h>
#include <encloud/setup.h>
#include <encloud/vpn.h>
#include "test.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_TOUT_MINS 5

static void state_cb (libencloud_state state, void *arg);

/**
 * Default is a "Pure C" libencloud test (no Qt objects): an internal
 * QCoreApplication is created if no instance is found!
 *
 * To run an external application define QTCORE_APP_EXTERNAL.
 */
int test_libencloud (int argc, char *argv[])
{
    TEST_TRACE;

    libencloud_rc rc = LIBENCLOUD_RC_SUCCESS;
    libencloud_t *libencloud = NULL;
#if 0
    libencloud_sb_info_t *sb_info;
    libencloud_vpn_conf_t *sb_conf;
#endif

#ifdef QTCORE_APP_EXTERNAL
    QCoreApplication a(argc, argv);
#endif

    TEST_ZERO ((rc = libencloud_create(argc, argv, &libencloud)));

#ifdef LIBENCLOUD_TYPE_SECE
    fprintf(stderr, "# type: SECE\n");

    // bad value - must fail
    TEST_ZERO ((rc = libencloud_setup_set_license(libencloud, "a-b-c-d")) == 0);

    // generate a random uuid
    TEST_ZERO ((rc = libencloud_setup_set_license(libencloud, qPrintable(QUuid::createUuid().toString()))));

    // variant() = 2 (Distributed Computing Environment), version() = 4 (Random-based)
    TEST_ZERO ((rc = libencloud_setup_set_license(libencloud, "67C8770B-44F1-410A-AB9A-F9B5446F13EE")));

    // variant() = 2 (Distributed Computing Environment), version() = 1 (Time-based)
    TEST_ZERO ((rc = libencloud_setup_set_license(libencloud, "{a8098c1a-f86e-11da-bd1a-00112444be1e}")));

#else
    fprintf(stderr, "# type: ECE\n");
    fprintf(stderr, "# serial: %s\n", libencloud_setup_get_serial(libencloud));
    fprintf(stderr, "# poi: %s\n", libencloud_setup_get_poi(libencloud));
#endif

    TEST_ZERO ((rc = libencloud_set_state_cb(libencloud, state_cb, NULL)));

    TEST_ZERO ((rc = libencloud_start(libencloud)));

    sleep(3);
    TEST_ZERO ((rc = libencloud_stop(libencloud)));
    sleep(3);
    TEST_ZERO ((rc = libencloud_start(libencloud)));
    sleep(3);
    TEST_ZERO ((rc = libencloud_stop(libencloud)));

#if 0
    TEST_ZERO_RETRY ((rc = libencloud_retr_sb_info(libencloud, &sb_info)), TEST_TOUT_MINS);
    TEST_ZERO_RETRY ((rc = libencloud_retr_sb_cert(libencloud)), TEST_TOUT_MINS);
    TEST_ZERO_RETRY ((rc = libencloud_retr_sb_conf(libencloud, &sb_conf)), TEST_TOUT_MINS);

    fprintf(stderr, "# license valid: %d\n", libencloud_sb_info_get_license_valid(sb_info));
    fprintf(stderr, "# license expiry: %ld\n", libencloud_sb_info_get_license_expiry(sb_info));
    fprintf(stderr, "# vpn ip: %s\n", libencloud_vpn_conf_get_vpn_ip(sb_conf));
    fprintf(stderr, "# vpn port: %d\n", libencloud_vpn_conf_get_vpn_port(sb_conf));
    fprintf(stderr, "# vpn proto: %s\n", libencloud_vpn_conf_get_vpn_proto(sb_conf));
    fprintf(stderr, "# vpn type: %s\n", libencloud_vpn_conf_get_vpn_type(sb_conf));
#endif

    libencloud_destroy(libencloud);

    return 0;

err:
    if (libencloud)
        libencloud_destroy(libencloud);

    fprintf (stderr, "# libencloud error (%d): %s\n", rc, libencloud_strerror(rc));

    return ~0;
}

static void state_cb (libencloud_state state, void *arg)
{
    fprintf(stderr, "# [%s] state: %d\n", __FUNCTION__, state);
}

#ifdef __cplusplus
}
#endif
