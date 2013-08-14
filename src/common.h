#ifndef _ECE_COMMON_H_
#define _ECE_COMMON_H_

namespace Ece {

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

} // namespace Ece

#endif
