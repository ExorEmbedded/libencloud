#ifndef _ECE_H_
#define _ECE_H_

#include <time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ECE return codes */
typedef enum
{
    ECE_RC_SUCCESS = 0,
    ECE_RC_BADPARAMS,
    ECE_RC_NOMEM,
    ECE_RC_NOLICENSE,
    ECE_RC_CONNECT,
    ECE_RC_BADAUTH,
    ECE_RC_TIMEOUT,
    ECE_RC_BADRESPONSE,
    ECE_RC_FAILED,
    ECE_RC_SYSERR,
    ECE_RC_GENERIC
} 
ece_rc_t;

/* Main ECE structure */
struct ece_s;
typedef struct ece_s ece_t;

/* SB Info structure */
struct ece_sb_info_s;
typedef struct ece_sb_info_s ece_sb_info_t;

/* SB Config structure */
struct ece_sb_conf_s;
typedef struct ece_sb_conf_s ece_sb_conf_t;

/* Constructor/Destructor */
ece_rc_t ece_create (int argc, char *argv[], ece_t **pece);
ece_rc_t ece_destroy (ece_t *ece);

/* Licensing */
ece_rc_t ece_set_license (ece_t *ece, const char *guid);

/* Communication */
ece_rc_t ece_retr_sb_info (ece_t *ece, ece_sb_info_t **pinfo);
ece_rc_t ece_retr_sb_cert (ece_t *ece);
ece_rc_t ece_retr_sb_conf (ece_t *ece, ece_sb_conf_t **pconf);

/* Accessor methods - ece_sb_info_t */
bool ece_sb_info_get_license_valid (ece_sb_info_t *info);
time_t ece_sb_info_get_license_expiry (ece_sb_info_t *info);

/* Accessor methods - ece_sb_conf_t */
char *ece_sb_conf_get_vpn_ip (ece_sb_conf_t *conf);
int ece_sb_conf_get_vpn_port (ece_sb_conf_t *conf);
char *ece_sb_conf_get_vpn_proto (ece_sb_conf_t *conf);
char *ece_sb_conf_get_vpn_type (ece_sb_conf_t *conf);

/* Other */
const char *ece_version ();
const char *ece_strerror (ece_rc_t rc);

#ifdef __cplusplus
}
#endif

#endif /* _ECE_H_ */
