#ifndef _LIBENCLOUD_VPN_H_
#define _LIBENCLOUD_VPN_H_

#include <encloud/common.h>

/* VPN Config structure */
struct LIBENCLOUD_DLLSPEC libencloud_vpn_conf_s;
typedef struct libencloud_vpn_conf_s libencloud_vpn_conf_t;

/* Accessor methods - libencloud_vpn_conf_t */
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_ip (libencloud_vpn_conf_t *conf);
LIBENCLOUD_DLLSPEC int libencloud_vpn_conf_get_vpn_port (libencloud_vpn_conf_t *conf);
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_proto (libencloud_vpn_conf_t *conf);
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_type (libencloud_vpn_conf_t *conf);

#endif /* _LIBENCLOUD_VPN_H_ */

