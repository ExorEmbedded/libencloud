#ifndef _ENCLOUD_CORE_H_
#define _ENCLOUD_CORE_H_

#include <encloud/common.h>
#include <encloud/state.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constructor/Destructor */
ENCLOUD_DLLSPEC encloud_rc encloud_create (int argc, char *argv[], encloud_t **pencloud);
ENCLOUD_DLLSPEC encloud_rc encloud_destroy (encloud_t *encloud);

/* Main settings */
ENCLOUD_DLLSPEC encloud_rc encloud_set_state_cb (encloud_t *encloud, encloud_state_cb state_cb);

/* Main control */
ENCLOUD_DLLSPEC encloud_rc encloud_start (encloud_t *encloud);
ENCLOUD_DLLSPEC encloud_rc encloud_loop (encloud_t *encloud);
ENCLOUD_DLLSPEC encloud_rc encloud_stop (encloud_t *encloud);

/* Other */
ENCLOUD_DLLSPEC const char *encloud_version (void);
ENCLOUD_DLLSPEC const char *encloud_revision (void);
ENCLOUD_DLLSPEC const char *encloud_strerror (encloud_rc rc);

#ifdef __cplusplus
}
#endif

#endif /* _ENCLOUD_CORE_H_ */
