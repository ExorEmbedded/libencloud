#include <common/common.h>
#include <common/config.h>
#include "setup.h"

namespace libencloud {

SetupInterface::SetupInterface (Config *cfg)
{
    _cfg = cfg;
}

} // namespace libencloud
