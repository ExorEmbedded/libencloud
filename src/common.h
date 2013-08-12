#ifndef _ECE_COMMON_H_
#define _ECE_COMMON_H_

namespace Ece {

/* Protocol modes */
enum ProtocolType
{
    ProtocolInitialization,
    ProtocolOperation
};

/* Message types */
enum MessageType
{
    MessageRetrInfo,
    MessageRetrCert,
    MessageRetrConf
};

} // namespace Ece

#endif
