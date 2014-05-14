#include <encloud/Client>
#include <common/common.h>
#include <common/config.h>
#include <common/message.h>

namespace libencloud {

//
// public methods
//

int MessageInterface::clear ()
{
    _cfg = NULL;
    _client = NULL;
    _data.clear();

    return 0;
}

int MessageInterface::setConfig (Config *cfg)
{
    LIBENCLOUD_ERR_IF (cfg == NULL);

    _cfg = cfg;

    return 0;
err:
    return ~0;
}

int MessageInterface::setClient (Client *client)
{
    LIBENCLOUD_ERR_IF (client == NULL);

    _client = client;

    return 0;
err:
    return ~0;
}

int MessageInterface::setData (const QVariant &data)
{
    _data = data;

    return 0;
}

} // namespace libencloud
