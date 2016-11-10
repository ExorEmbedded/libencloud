#include <common/common.h>
#include <common/config.h>
#include "setup.h"

namespace libencloud {

SetupInterface::SetupInterface (Config *cfg)
    : _qnam(NULL)
{
    _cfg = cfg;
}

QNetworkAccessManager *SetupInterface::getNetworkAccessManager ()
{
    return _qnam;
}

int SetupInterface::setNetworkAccessManager (QNetworkAccessManager *qnam)
{
    LIBENCLOUD_RETURN_IF (qnam == NULL, ~0);

    _qnam = qnam;

    return 0;
}

} // namespace libencloud
