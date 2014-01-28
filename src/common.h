#ifndef _ENCLOUD_PRIV_COMMON_H_
#define _ENCLOUD_PRIV_COMMON_H_

#include "defaults.h"
#include "helpers.h"

namespace encloud {

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

} // namespace encloud

#endif  /* _ENCLOUD_PRIV_COMMON_H_ */
