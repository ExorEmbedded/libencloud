#include <common/common.h>
#include <common/config.h>
#include <common/message.h>
#include <common/client.h>

namespace libencloud {

void MessageInterface::setConfig (Config *cfg)
{
    _cfg = cfg;
}

void MessageInterface::setClient (Client *client)
{
    _client = client;
}

} // namespace libencloud
