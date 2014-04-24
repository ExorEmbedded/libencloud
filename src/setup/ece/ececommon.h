#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_COMMON_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_COMMON_H_

#include <QUrl>
#include <QSslConfiguration>
#include <common/config.h>

// use only to wrap upper-level methods, otherwise duplicates will be emitted
#define EMIT_ERROR_ERR_IF(cond) \
    LIBENCLOUD_EMIT_ERR_IF(cond, error(Error(Error::CodeGeneric)))

namespace libencloud {
namespace setupece {

/* Protocol modes */
typedef enum
{
    ProtocolTypeInit,
    ProtocolTypeOp
} 
ProtocolType;
#define LIBENCLOUD_SETUPECE_PROTOCOLTYPE_VALID(p) (p >= ProtocolTypeInit && p <= ProtocolTypeOp)

int loadSslConfig (ProtocolType proto, Config *cfg, QUrl &url, QSslConfiguration &sslcfg);

} // namespace setupece
} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_COMMON_H_ */


