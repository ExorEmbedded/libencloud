#ifndef _ENCLOUD_VPN_H_
#define _ENCLOUD_VPN_H_

#include <encloud/common.h>

/* VPN Config structure */
struct ENCLOUD_DLLSPEC encloud_vpn_conf_s;
typedef struct encloud_vpn_conf_s encloud_vpn_conf_t;

/* Accessor methods - encloud_vpn_conf_t */
ENCLOUD_DLLSPEC char *encloud_vpn_conf_get_vpn_ip (encloud_vpn_conf_t *conf);
ENCLOUD_DLLSPEC int encloud_vpn_conf_get_vpn_port (encloud_vpn_conf_t *conf);
ENCLOUD_DLLSPEC char *encloud_vpn_conf_get_vpn_proto (encloud_vpn_conf_t *conf);
ENCLOUD_DLLSPEC char *encloud_vpn_conf_get_vpn_type (encloud_vpn_conf_t *conf);

#endif /* _ENCLOUD_VPN_H_ */

