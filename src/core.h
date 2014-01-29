#ifndef _ENCLOUD_PRIV_CORE_H_
#define _ENCLOUD_PRIV_CORE_H_

#include <encloud/common.h>
#include "common.h"
#include "config.h"
#include "context.h"

/** \brief ENCLOUD internal object */
struct encloud_s
{
    encloud::Context *context;
};

#endif  /* _ENCLOUD_PRIV_CORE_H_ */
