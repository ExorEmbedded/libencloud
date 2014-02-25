#ifndef _LIBENCLOUD_PRIV_HTTP_HANDLER_H_
#define _LIBENCLOUD_PRIV_HTTP_HANDLER_H_

#include <common/common.h>
#include <common/config.h>

// include all API versions from here
#include "handler1.h"

#define LIBENCLOUD_HANDLER_PATH_REGEX          "/api_v(\\d+)/(.*)"

#define LIBENCLOUD_HANDLER_ERR_IF(cond,status) \
    do { \
        if (cond) { \
            response.setStatus(status); \
            LIBENCLOUD_ERR_IF (1); \
        } \
    } while (0);

#define LIBENCLOUD_HANDLER_OK response.setStatus(LIBENCLOUD_HTTP_STATUS_OK)
#define LIBENCLOUD_HANDLER_STATUS(status) response.setStatus(status)

#endif  /* _LIBENCLOUD_PRIV_HTTP_HANDLER_H_ */
