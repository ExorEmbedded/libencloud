#ifndef _ENCLOUD_COMMON_H_
#define _ENCLOUD_COMMON_H_

#ifndef ENCLOUD_DLLSPEC
#  if defined(Q_OS_WIN)
#    if defined _ENCLOUDLIB_
#       define ENCLOUD_DLLSPEC __declspec(dllexport)
#    else
#        define ENCLOUD_DLLSPEC __declspec(dllimport)
#    endif
#else
#    define ENCLOUD_DLLSPEC
#endif
#endif

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

#endif
