#ifndef _ENCLOUD_PRIV_CORE_H_
#define _ENCLOUD_PRIV_CORE_H_

#include <QtCore/QCoreApplication>
#include <encloud/common.h>
#include "client.h"
#include "vpn.h"
#include "crypto.h"
#include "config.h"

/** \brief SB info object */
struct encloud_info_s
{
    bool license_valid;
    time_t license_expiry;
};
typedef struct encloud_info_s encloud_info_t;

/** \brief ENCLOUD internal object */
struct encloud_s
{
    QCoreApplication *app;

#ifndef ENCLOUD_TYPE_SECE
    char *serial;
    char *poi;
#endif

    encloud::Client *client;
    encloud::Config *cfg;

    encloud_crypto_t crypto;
    encloud_info_t info;
    encloud_vpn_conf_t conf;
};

#endif  /* _ENCLOUD_PRIV_VPN_H_ */

