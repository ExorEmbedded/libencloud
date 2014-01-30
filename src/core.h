#ifndef _LIBENCLOUD_PRIV_CORE_H_
#define _LIBENCLOUD_PRIV_CORE_H_

#include <encloud/common.h>
#include "common.h"
#include "config.h"
#include "context.h"

/** \brief LIBENCLOUD internal object */
struct libencloud_s
{
    libencloud::Context *context;
};

#endif  /* _LIBENCLOUD_PRIV_CORE_H_ */
