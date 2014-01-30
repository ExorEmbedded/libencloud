#ifndef _LIBENCLOUD_SETUP_H_
#define _LIBENCLOUD_SETUP_H_

#include <encloud/common.h>

#ifndef LIBENCLOUD_DISABLE_SETUP

#ifdef LIBENCLOUD_TYPE_SECE
/* SECE: Licensing */
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_setup_set_license (libencloud_t *libencloud, const char *guid);
#else
/* ECE: Identification */
LIBENCLOUD_DLLSPEC const char *libencloud_setup_get_serial (libencloud_t *libencloud);  /* optional if CN provided in CSR template */
LIBENCLOUD_DLLSPEC const char *libencloud_setup_get_poi (libencloud_t *libencloud);
#endif

#endif /* LIBENCLOUD_DISABLE_SETUP */

#endif /* _LIBENCLOUD_SETUP_H_ */
