#ifndef _ECE_H_
#define _ECE_H_

#ifndef ECE_DLLSPEC
#  if defined(ECE_WIN32)
#    if defined _ECELIB_
#       define ECE_DLLSPEC __declspec(dllexport)
#    else
#       define ECE_DLLSPEC __declspec(dllimport)
#    endif
#  else
#    define ECE_DLLSPEC
#  endif
#endif

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
    ECE_RC_BADCONFIG,
    ECE_RC_NOLICENSE,
    ECE_RC_CONNECT,
    ECE_RC_BADAUTH,
    ECE_RC_TIMEOUT,
    ECE_RC_BADRESPONSE,
    ECE_RC_INVALIDCERT,
    ECE_RC_FAILED,
    ECE_RC_SYSERR,
    ECE_RC_GENERIC
} 
ece_rc_t;

/* Main ECE structure */
struct ECE_DLLSPEC ece_s;
typedef struct ece_s ece_t;

/* SB Info structure */
struct ECE_DLLSPEC ece_sb_info_s;
typedef struct ece_sb_info_s ece_sb_info_t;

/* SB Config structure */
struct ECE_DLLSPEC ece_sb_conf_s;
typedef struct ece_sb_conf_s ece_sb_conf_t;

/* Constructor/Destructor */
ECE_DLLSPEC ece_rc_t ece_create (int argc, char *argv[], ece_t **pece);
ECE_DLLSPEC ece_rc_t ece_destroy (ece_t *ece);

/* Licensing */
ECE_DLLSPEC ece_rc_t ece_set_license (ece_t *ece, const char *guid);

/* Communication */
ECE_DLLSPEC ece_rc_t ece_retr_sb_info (ece_t *ece, ece_sb_info_t **pinfo);
ECE_DLLSPEC ece_rc_t ece_retr_sb_cert (ece_t *ece);
ECE_DLLSPEC ece_rc_t ece_retr_sb_conf (ece_t *ece, ece_sb_conf_t **pconf);

/* Accessor methods - ece_sb_info_t */
ECE_DLLSPEC bool ece_sb_info_get_license_valid (ece_sb_info_t *info);
ECE_DLLSPEC time_t ece_sb_info_get_license_expiry (ece_sb_info_t *info);

/* Accessor methods - ece_sb_conf_t */
ECE_DLLSPEC char *ece_sb_conf_get_vpn_ip (ece_sb_conf_t *conf);
ECE_DLLSPEC int ece_sb_conf_get_vpn_port (ece_sb_conf_t *conf);
ECE_DLLSPEC char *ece_sb_conf_get_vpn_proto (ece_sb_conf_t *conf);
ECE_DLLSPEC char *ece_sb_conf_get_vpn_type (ece_sb_conf_t *conf);

/* Other */
ECE_DLLSPEC const char *ece_version (void);
ECE_DLLSPEC const char *ece_revision (void);
ECE_DLLSPEC const char *ece_strerror (ece_rc_t rc);

#ifdef __cplusplus
}
#endif

#endif /* _ECE_H_ */
