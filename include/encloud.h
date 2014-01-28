#ifndef _ENCLOUD_H_
#define _ENCLOUD_H_

#ifndef ENCLOUD_DLLSPEC
#  if defined(ENCLOUD_WIN32)
#    if defined _ENCLOUDLIB_
#       define ENCLOUD_DLLSPEC __declspec(dllexport)
#    else
#       define ENCLOUD_DLLSPEC __declspec(dllimport)
#    endif
#  else
#    define ENCLOUD_DLLSPEC
#  endif
#endif

#include <time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ENCLOUD return codes */
typedef enum
{
    ENCLOUD_RC_SUCCESS = 0,
    ENCLOUD_RC_BADPARAMS,
    ENCLOUD_RC_NOMEM,
    ENCLOUD_RC_BADCONFIG,
    ENCLOUD_RC_NOLICENSE,
    ENCLOUD_RC_CONNECT,
    ENCLOUD_RC_BADAUTH,
    ENCLOUD_RC_TIMEOUT,
    ENCLOUD_RC_BADRESPONSE,
    ENCLOUD_RC_INVALIDCERT,
    ENCLOUD_RC_FAILED,
    ENCLOUD_RC_SYSERR,
    ENCLOUD_RC_GENERIC
}
encloud_rc_t;

/* Main ENCLOUD structure */
struct ENCLOUD_DLLSPEC encloud_s;
typedef struct encloud_s encloud_t;

/* SB Info structure */
struct ENCLOUD_DLLSPEC encloud_sb_info_s;
typedef struct encloud_sb_info_s encloud_sb_info_t;

/* SB Config structure */
struct ENCLOUD_DLLSPEC encloud_sb_conf_s;
typedef struct encloud_sb_conf_s encloud_sb_conf_t;

/* Constructor/Destructor */
ENCLOUD_DLLSPEC encloud_rc_t encloud_create (int argc, char *argv[], encloud_t **pencloud);
ENCLOUD_DLLSPEC encloud_rc_t encloud_destroy (encloud_t *encloud);

#ifdef ENCLOUD_TYPE_SECE
/* SECE: Licensing */
ENCLOUD_DLLSPEC encloud_rc_t encloud_set_license (encloud_t *encloud, const char *guid);
#else
/* ENCLOUD: Identification */
ENCLOUD_DLLSPEC const char *encloud_get_serial (encloud_t *encloud);  /* optional if CN provided in CSR template */
ENCLOUD_DLLSPEC const char *encloud_get_poi (encloud_t *encloud);
#endif

/* Communication */
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_info (encloud_t *encloud, encloud_sb_info_t **pinfo);
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_cert (encloud_t *encloud);
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_conf (encloud_t *encloud, encloud_sb_conf_t **pconf);

/* Accessor methods - encloud_sb_info_t */
/* SECE: based license, ENCLOUD: based on op_cert validity/expiry */
ENCLOUD_DLLSPEC bool encloud_sb_info_get_license_valid (encloud_sb_info_t *info);
ENCLOUD_DLLSPEC time_t encloud_sb_info_get_license_expiry (encloud_sb_info_t *info);

/* Accessor methods - encloud_sb_conf_t */
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_ip (encloud_sb_conf_t *conf);
ENCLOUD_DLLSPEC int encloud_sb_conf_get_vpn_port (encloud_sb_conf_t *conf);
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_proto (encloud_sb_conf_t *conf);
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_type (encloud_sb_conf_t *conf);

/* Other */
ENCLOUD_DLLSPEC const char *encloud_version (void);
ENCLOUD_DLLSPEC const char *encloud_revision (void);
ENCLOUD_DLLSPEC const char *encloud_strerror (encloud_rc_t rc);

#ifdef __cplusplus
}
#endif

#endif /* _ENCLOUD_H_ */
