#ifndef _LIBENCLOUD_PRIV_VPN_H_
#define _LIBENCLOUD_PRIV_VPN_H_

#include "defaults.h"

/** \brief SB configuration object */
struct libencloud_vpn_conf_s
{
    char vpn_ip[LIBENCLOUD_DESC_SZ];
    int vpn_port;
    char vpn_proto[LIBENCLOUD_DESC_SZ];
    char vpn_type[LIBENCLOUD_DESC_SZ];
};
typedef struct libencloud_vpn_conf_s libencloud_vpn_conf_t;

#endif  /* _LIBENCLOUD_PRIV_VPN_H_ */
