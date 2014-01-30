#ifndef _LIBENCLOUD_CORE_H_
#define _LIBENCLOUD_CORE_H_

#include <encloud/common.h>
#include <encloud/state.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constructor/Destructor */
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_create (int argc, char *argv[], libencloud_t **plibencloud);
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_destroy (libencloud_t *libencloud);

/* Main settings */
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_set_state_cb (libencloud_t *libencloud, libencloud_state_cb state_cb, void *arg);

/* Main control */
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_start (libencloud_t *libencloud);
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_stop (libencloud_t *libencloud);

/* Other */
LIBENCLOUD_DLLSPEC const char *libencloud_version (void);
LIBENCLOUD_DLLSPEC const char *libencloud_revision (void);
LIBENCLOUD_DLLSPEC const char *libencloud_strerror (libencloud_rc rc);

#ifdef __cplusplus
}
#endif

#endif /* _LIBENCLOUD_CORE_H_ */
