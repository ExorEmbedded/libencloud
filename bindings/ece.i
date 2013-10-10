%module ece
%{
#include <ece.h>
%}

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
struct ECE_DLLSPEC ece_s;
typedef struct ece_s ece_t;

/* SB Info structure */
struct ECE_DLLSPEC ece_sb_info_s;
typedef struct ece_sb_info_s ece_sb_info_t;

/* SB Config structure */
struct ECE_DLLSPEC ece_sb_conf_s;
typedef struct ece_sb_conf_s ece_sb_conf_t;

/* ece_t type remappings */
%typemap(in, numinputs=0) ece_t ** (ece_t **pece) {
    $1 = (ece_t **) &pece;
}
%typemap(argout) ece_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed ece_create() (rc=%d: %s)",
                result, ece_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_ece_s, 0);
}

/* ece_sb_info_t type remappings */
%typemap(in, numinputs=0) ece_sb_info_t ** (ece_sb_info_t **pece) {
    $1 = (ece_sb_info_t **) &pece;
}
%typemap(argout) ece_sb_info_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed ece_retr_sb_info() (rc=%d: %s)",
                result, ece_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_ece_sb_info_s, 0);
}

/* ece_sb_conf_t type remappings */
%typemap(in, numinputs=0) ece_sb_conf_t ** (ece_sb_conf_t **pece) {
    $1 = (ece_sb_conf_t **) &pece;
}
%typemap(argout) ece_sb_conf_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed ece_retr_sb_conf() (rc=%d: %s)",
                result, ece_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_ece_sb_conf_s, 0);
}

/* Constructor/Destructor */
%rename ece_create create;
ECE_DLLSPEC ece_rc_t ece_create (int argc, char *argv[], ece_t **pece);

%rename ece_destroy destroy;
ECE_DLLSPEC ece_rc_t ece_destroy (ece_t *ece);

/* Licensing */
%rename ece_set_license set_license;
ECE_DLLSPEC ece_rc_t ece_set_license (ece_t *ece, const char *guid);

/* Communication */
%rename ece_retr_sb_info retr_sb_info;
ECE_DLLSPEC ece_rc_t ece_retr_sb_info (ece_t *ece, ece_sb_info_t **pinfo);

%rename ece_retr_sb_cert retr_sb_cert;
ECE_DLLSPEC ece_rc_t ece_retr_sb_cert (ece_t *ece);

%rename ece_retr_sb_conf retr_sb_conf;
ECE_DLLSPEC ece_rc_t ece_retr_sb_conf (ece_t *ece, ece_sb_conf_t **pconf);

/* Accessor methods - ece_sb_info_t */
%rename ece_sb_info_get_license_valid sb_info_get_license_valid;
ECE_DLLSPEC bool ece_sb_info_get_license_valid (ece_sb_info_t *info);

%rename ece_sb_info_get_license_expiry sb_info_get_license_expiry;
ECE_DLLSPEC time_t ece_sb_info_get_license_expiry (ece_sb_info_t *info);

/* Accessor methods - ece_sb_conf_t */
%rename ece__sb_conf_get_vpn_ip _sb_conf_get_vpn_ip;
ECE_DLLSPEC char *ece_sb_conf_get_vpn_ip (ece_sb_conf_t *conf);

%rename ece_sb_conf_get_vpn_port sb_conf_get_vpn_port;
ECE_DLLSPEC int ece_sb_conf_get_vpn_port (ece_sb_conf_t *conf);

%rename ece_sb_conf_get_vpn_proto sb_conf_get_vpn_proto;
ECE_DLLSPEC char *ece_sb_conf_get_vpn_proto (ece_sb_conf_t *conf);

%rename ece_sb_conf_get_vpn_type sb_conf_get_vpn_type;
ECE_DLLSPEC char *ece_sb_conf_get_vpn_type (ece_sb_conf_t *conf);

/* Other */
%rename ece_version version;
ECE_DLLSPEC const char *ece_version (void);

%rename ece_revision revision;
ECE_DLLSPEC const char *ece_revision (void);

%rename ece_strerror strerror;
ECE_DLLSPEC const char *ece_strerror (ece_rc_t rc);

#ifdef __cplusplus
}
#endif
