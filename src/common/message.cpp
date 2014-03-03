#include <encloud/Client>
#include <common/common.h>
#include <common/config.h>
#include <common/message.h>

namespace libencloud {

//
// public methods
//

void MessageInterface::setConfig (Config *cfg)
{
    _cfg = cfg;
}

void MessageInterface::setClient (Client *client)
{
    _client = client;
}

} // namespace libencloud
