#include <encloud/vpn.h>
#include "common.h"
#include "config.h"
#include "vpn.h"

/** \brief Get VPN IP address */
ENCLOUD_DLLSPEC char *encloud_vpn_conf_get_vpn_ip (encloud_vpn_conf_t *conf)
{
    ENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_ip;
}

/** \brief Get VPN port */
ENCLOUD_DLLSPEC int encloud_vpn_conf_get_vpn_port (encloud_vpn_conf_t *conf)
{
    ENCLOUD_RETURN_IF (conf == NULL, -1);

    return conf->vpn_port;
}

/** \brief Get VPN protocol */
ENCLOUD_DLLSPEC char *encloud_vpn_conf_get_vpn_proto (encloud_vpn_conf_t *conf)
{
    ENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_proto;
}

/** \brief Get VPN type */
ENCLOUD_DLLSPEC char *encloud_vpn_conf_get_vpn_type (encloud_vpn_conf_t *conf)
{
    ENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_type;
}
