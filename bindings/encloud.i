%module encloud
%{
#include <encloud.h>
%}

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

/* encloud_t type remappings */
%typemap(in, numinputs=0) encloud_t ** (encloud_t **pencloud) {
    $1 = (encloud_t **) &pencloud;
}
%typemap(argout) encloud_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed encloud_create() (rc=%d: %s)",
                result, encloud_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_encloud_s, 0);
}

/* encloud_sb_info_t type remappings */
%typemap(in, numinputs=0) encloud_sb_info_t ** (encloud_sb_info_t **pencloud) {
    $1 = (encloud_sb_info_t **) &pencloud;
}
%typemap(argout) encloud_sb_info_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed encloud_retr_sb_info() (rc=%d: %s)",
                result, encloud_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_encloud_sb_info_s, 0);
}

/* encloud_sb_conf_t type remappings */
%typemap(in, numinputs=0) encloud_sb_conf_t ** (encloud_sb_conf_t **pencloud) {
    $1 = (encloud_sb_conf_t **) &pencloud;
}
%typemap(argout) encloud_sb_conf_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed encloud_retr_sb_conf() (rc=%d: %s)",
                result, encloud_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_encloud_sb_conf_s, 0);
}

/* Constructor/Destructor */
%rename encloud_create create;
ENCLOUD_DLLSPEC encloud_rc_t encloud_create (int argc, char *argv[], encloud_t **pencloud);

%rename encloud_destroy destroy;
ENCLOUD_DLLSPEC encloud_rc_t encloud_destroy (encloud_t *encloud);

/* Licensing */
#ifdef ENCLOUD_TYPE_SECE
/* SECE: Licensing */
%rename encloud_set_license set_license;
ENCLOUD_DLLSPEC encloud_rc_t encloud_set_license (encloud_t *encloud, const char *guid);
#else
/* ENCLOUD: Identification */
%rename encloud_get_serial get_serial;
ENCLOUD_DLLSPEC const char *encloud_get_serial (encloud_t *encloud);

%rename encloud_get_poi get_poi;
ENCLOUD_DLLSPEC const char *encloud_get_poi (encloud_t *encloud);
#endif

/* Communication */
%rename encloud_retr_sb_info retr_sb_info;
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_info (encloud_t *encloud, encloud_sb_info_t **pinfo);

%rename encloud_retr_sb_cert retr_sb_cert;
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_cert (encloud_t *encloud);

%rename encloud_retr_sb_conf retr_sb_conf;
ENCLOUD_DLLSPEC encloud_rc_t encloud_retr_sb_conf (encloud_t *encloud, encloud_sb_conf_t **pconf);

/* Accessor methods - encloud_sb_info_t */
%rename encloud_sb_info_get_license_valid sb_info_get_license_valid;
ENCLOUD_DLLSPEC bool encloud_sb_info_get_license_valid (encloud_sb_info_t *info);

%rename encloud_sb_info_get_license_expiry sb_info_get_license_expiry;
ENCLOUD_DLLSPEC time_t encloud_sb_info_get_license_expiry (encloud_sb_info_t *info);

/* Accessor methods - encloud_sb_conf_t */
%rename encloud_sb_conf_get_vpn_ip sb_conf_get_vpn_ip;
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_ip (encloud_sb_conf_t *conf);

%rename encloud_sb_conf_get_vpn_port sb_conf_get_vpn_port;
ENCLOUD_DLLSPEC int encloud_sb_conf_get_vpn_port (encloud_sb_conf_t *conf);

%rename encloud_sb_conf_get_vpn_proto sb_conf_get_vpn_proto;
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_proto (encloud_sb_conf_t *conf);

%rename encloud_sb_conf_get_vpn_type sb_conf_get_vpn_type;
ENCLOUD_DLLSPEC char *encloud_sb_conf_get_vpn_type (encloud_sb_conf_t *conf);

/* Other */
%rename encloud_version version;
ENCLOUD_DLLSPEC const char *encloud_version (void);

%rename encloud_revision revision;
ENCLOUD_DLLSPEC const char *encloud_revision (void);

%rename encloud_strerror strerror;
ENCLOUD_DLLSPEC const char *encloud_strerror (encloud_rc_t rc);

#ifdef __cplusplus
}
#endif
