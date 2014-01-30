#ifndef _LIBENCLOUD_COMMON_H_
#define _LIBENCLOUD_COMMON_H_

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
#include <encloud/stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* libencloud return codes */
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

/* Main libencloud structure */
struct LIBENCLOUD_DLLSPEC libencloud_s;
typedef struct libencloud_s libencloud_t;

#ifdef __cplusplus
}
#endif

#endif /* _LIBENCLOUD_COMMON_H_ */
