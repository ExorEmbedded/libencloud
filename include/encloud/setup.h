#ifndef _ENCLOUD_SETUP_H_
#define _ENCLOUD_SETUP_H_

#include <encloud/common.h>

#ifndef ENCLOUD_DISABLE_SETUP

#ifdef ENCLOUD_TYPE_SECE
/* SECE: Licensing */
ENCLOUD_DLLSPEC encloud_rc encloud_setup_set_license (encloud_t *encloud, const char *guid);
#else
/* ECE: Identification */
ENCLOUD_DLLSPEC const char *encloud_setup_get_serial (encloud_t *encloud);  /* optional if CN provided in CSR template */
ENCLOUD_DLLSPEC const char *encloud_setup_get_poi (encloud_t *encloud);
#endif

#endif /* ENCLOUD_DISABLE_SETUP */

#endif /* _ENCLOUD_SETUP_H_ */
