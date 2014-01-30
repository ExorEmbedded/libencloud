#include <encloud/vpn.h>
#include "common.h"
#include "config.h"
#include "vpn.h"

/** \brief Get VPN IP address */
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_ip (libencloud_vpn_conf_t *conf)
{
    LIBENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_ip;
}

/** \brief Get VPN port */
LIBENCLOUD_DLLSPEC int libencloud_vpn_conf_get_vpn_port (libencloud_vpn_conf_t *conf)
{
    LIBENCLOUD_RETURN_IF (conf == NULL, -1);

    return conf->vpn_port;
}

/** \brief Get VPN protocol */
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_proto (libencloud_vpn_conf_t *conf)
{
    LIBENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_proto;
}

/** \brief Get VPN type */
LIBENCLOUD_DLLSPEC char *libencloud_vpn_conf_get_vpn_type (libencloud_vpn_conf_t *conf)
{
    LIBENCLOUD_RETURN_IF (conf == NULL, NULL);

    return conf->vpn_type;
}
