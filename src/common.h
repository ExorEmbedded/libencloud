#ifndef _LIBENCLOUD_PRIV_COMMON_H_
#define _LIBENCLOUD_PRIV_COMMON_H_

#include "defaults.h"
#include "helpers.h"

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
