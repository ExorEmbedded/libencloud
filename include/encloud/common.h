#ifndef _ENCLOUD_COMMON_H_
#define _ENCLOUD_COMMON_H_

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
#include <encloud/stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* libencloud return codes */
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
encloud_rc;

/* Main libencloud structure */
struct ENCLOUD_DLLSPEC encloud_s;
typedef struct encloud_s encloud_t;

#ifdef __cplusplus
}
#endif

#endif /* _ENCLOUD_COMMON_H_ */
