#ifndef _SECE_COMMON_H_
#define _SECE_COMMON_H_

#include <encloud/Common>
#include "helpers.h"

#define SECE_APP                "SECE"
#define SECE_STATUS_PERIOD      1000

// don't use external service by default (inbuilt Encloud - requires root
// privileges) comment this to use external Encloud service
#define SECE_DISABLE_SERVICE

#endif  // _SECE_COMMON_H_
