#ifndef _ECE_H_
#define _ECE_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ECE_RC_SUCCESS = 0,
    ECE_RC_BADPARAMS,
    ECE_RC_NOMEM,
    ECE_RC_CONNECT,
    ECE_RC_BADAUTH,
    ECE_RC_TIMEOUT,
    ECE_RC_FAILED,
    ECE_RC_GENERIC
} 
ece_rc_t;

struct ece_s;
typedef struct ece_s ece_t;

struct ece_sb_info_s;
typedef struct ece_sb_info_s ece_sb_info_t;

/* Constructor/Destructor */
ece_rc_t ece_create (int argc, char *argv[], ece_t **ece);
ece_rc_t ece_destroy (ece_t *ece);

/* Licensing */
ece_rc_t ece_set_license (ece_t *ece, const char *guid);

/* Communication */
ece_rc_t ece_retr_sb_info (ece_t *ece, ece_sb_info_t **info);
ece_rc_t ece_retr_sb_cert (ece_t *ece);
ece_rc_t ece_retr_sb_conf (ece_t *ece);

/* Accessor methods - ece_sb_info_t */
bool ece_sb_info_get_license_valid (ece_sb_info_t *info);
time_t ece_sb_info_get_license_expiry (ece_sb_info_t *ece);

/* Other */
const char *ece_version ();
const char *ece_strerror (ece_rc_t rc);

#ifdef __cplusplus
}
#endif

#endif /* _ECE_H_ */
