#ifndef _ENCLOUD_PRIV_VPN_H_
#define _ENCLOUD_PRIV_VPN_H_

#include "defaults.h"

/** \brief SB configuration object */
struct encloud_vpn_conf_s
{
    char vpn_ip[ENCLOUD_DESC_SZ];
    int vpn_port;
    char vpn_proto[ENCLOUD_DESC_SZ];
    char vpn_type[ENCLOUD_DESC_SZ];
};
typedef struct encloud_vpn_conf_s encloud_vpn_conf_t;

#endif  /* _ENCLOUD_PRIV_VPN_H_ */
