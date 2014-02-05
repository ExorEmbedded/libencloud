%module encloud
%{
#include <encloud/Common>
%}

#ifndef LIBENCLOUD_DLLSPEC
#  if defined(LIBENCLOUD_WIN32)
#    if defined _LIBENCLOUDLIB_
#       define LIBENCLOUD_DLLSPEC __declspec(dllexport)
#    else
#       define LIBENCLOUD_DLLSPEC __declspec(dllimport)
#    endif
#  else
#    define LIBENCLOUD_DLLSPEC
#  endif
#endif

#include <time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* LIBENCLOUD return codes */
typedef enum
{
    LIBENCLOUD_RC_SUCCESS = 0,
    LIBENCLOUD_RC_BADPARAMS,
    LIBENCLOUD_RC_NOMEM,
    LIBENCLOUD_RC_BADCONFIG,
    LIBENCLOUD_RC_NOLICENSE,
    LIBENCLOUD_RC_CONNECT,
    LIBENCLOUD_RC_BADAUTH,
    LIBENCLOUD_RC_TIMEOUT,
    LIBENCLOUD_RC_BADRESPONSE,
    LIBENCLOUD_RC_INVALIDCERT,
    LIBENCLOUD_RC_FAILED,
    LIBENCLOUD_RC_SYSERR,
    LIBENCLOUD_RC_GENERIC
}
libencloud_rc;

/* Main LIBENCLOUD structure */
struct LIBENCLOUD_DLLSPEC libencloud_s;
typedef struct libencloud_s libencloud_t;

/* SB Info structure */
struct LIBENCLOUD_DLLSPEC libencloud_sb_info_s;
typedef struct libencloud_sb_info_s libencloud_sb_info_t;

/* SB Config structure */
struct LIBENCLOUD_DLLSPEC libencloud_vpn_conf_s;
typedef struct libencloud_vpn_conf_s libencloud_vpn_conf_t;

/* libencloud_t type remappings */
%typemap(in, numinputs=0) libencloud_t ** (libencloud_t **plibencloud) {
    $1 = (libencloud_t **) &plibencloud;
}
%typemap(argout) libencloud_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed libencloud_create() (rc=%d: %s)",
                result, libencloud_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_libencloud_s, 0);
}

/* libencloud_sb_info_t type remappings */
%typemap(in, numinputs=0) libencloud_sb_info_t ** (libencloud_sb_info_t **plibencloud) {
    $1 = (libencloud_sb_info_t **) &plibencloud;
}
%typemap(argout) libencloud_sb_info_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed libencloud_retr_sb_info() (rc=%d: %s)",
                result, libencloud_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_libencloud_sb_info_s, 0);
}

/* libencloud_vpn_conf_t type remappings */
%typemap(in, numinputs=0) libencloud_vpn_conf_t ** (libencloud_vpn_conf_t **plibencloud) {
    $1 = (libencloud_vpn_conf_t **) &plibencloud;
}
%typemap(argout) libencloud_vpn_conf_t ** {
    char buf[128];
    if (result) {
        snprintf(buf, sizeof(buf), "failed libencloud_retr_sb_conf() (rc=%d: %s)",
                result, libencloud_strerror(result));
        SWIG_exception_fail(0, buf);
    }
    $result = SWIG_NewPointerObj(*$1, SWIGTYPE_p_libencloud_vpn_conf_s, 0);
}

/* Constructor/Destructor */
%rename libencloud_create create;
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_create (int argc, char *argv[], libencloud_t **plibencloud);

%rename libencloud_destroy destroy;
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_destroy (libencloud_t *libencloud);

/* Main control */
%rename libencloud_start start;
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_start (libencloud_t *libencloud);

%rename libencloud_stop stop;
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_stop (libencloud_t *libencloud);

#if 0
/* Licensing */
#ifdef LIBENCLOUD_MODE_SECE
/* SECE: Licensing */
%rename libencloud_setup_set_license set_license;
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_setup_set_license (libencloud_t *libencloud, const char *guid);
#else
/* LIBENCLOUD: Identification */
%rename libencloud_setup_get_serial get_serial;
LIBENCLOUD_DLLSPEC const char *libencloud_setup_get_serial (libencloud_t *libencloud);

%rename libencloud_setup_get_poi get_poi;
LIBENCLOUD_DLLSPEC const char *libencloud_setup_get_poi (libencloud_t *libencloud);
#endif

/* Communication */
%rename libencloud_retr_sb_info retr_sb_info;
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_retr_sb_info (libencloud_t *libencloud, libencloud_sb_info_t **pinfo);

%rename libencloud_retr_sb_cert retr_sb_cert;
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_retr_sb_cert (libencloud_t *libencloud);

%rename libencloud_retr_sb_conf retr_sb_conf;
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_retr_sb_conf (libencloud_t *libencloud, libencloud_vpn_conf_t **pconf);

/* Accessor methods - libencloud_sb_info_t */
%rename libencloud_sb_info_get_license_valid sb_info_get_license_valid;
LIBENCLOUD_DLLSPEC bool libencloud_sb_info_get_license_valid (libencloud_sb_info_t *info);

%rename libencloud_sb_info_get_license_expiry sb_info_get_license_expiry;
LIBENCLOUD_DLLSPEC time_t libencloud_sb_info_get_license_expiry (libencloud_sb_info_t *info);

/* Accessor methods - libencloud_vpn_conf_t */
%rename libencloud_vpn_conf_get_vpn_ip sb_conf_get_vpn_ip;
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_ip (libencloud_vpn_conf_t *conf);

%rename libencloud_vpn_conf_get_vpn_port sb_conf_get_vpn_port;
LIBENCLOUD_DLLSPEC int libencloud_vpn_conf_get_vpn_port (libencloud_vpn_conf_t *conf);

%rename libencloud_vpn_conf_get_vpn_proto sb_conf_get_vpn_proto;
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_proto (libencloud_vpn_conf_t *conf);

%rename libencloud_vpn_conf_get_vpn_type sb_conf_get_vpn_type;
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_type (libencloud_vpn_conf_t *conf);
#endif

/* Other */
%rename libencloud_version version;
LIBENCLOUD_DLLSPEC const char *libencloud_version (void);

%rename libencloud_revision revision;
LIBENCLOUD_DLLSPEC const char *libencloud_revision (void);

%rename libencloud_strerror strerror;
LIBENCLOUD_DLLSPEC const char *libencloud_strerror (libencloud_rc rc);

#ifdef __cplusplus
}
#endif
