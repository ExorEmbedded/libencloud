#ifndef _LIBENCLOUD_PRIV_COMMON_H_
#define _LIBENCLOUD_PRIV_COMMON_H_

#include <common/defaults.h>
#include <common/helpers.h>

namespace libencloud {

/* Protocol modes */
enum ProtocolType
{
    ProtocolTypeInit,
    ProtocolTypeOp
};

/* Message types */
enum MessageType
{
    MessageTypeRetrInfo,
    MessageTypeRetrCert,
    MessageTypeRetrConf
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_COMMON_H_ */
