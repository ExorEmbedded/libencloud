#include <encloud/setup.h>
#include "core.h"
#include "common.h"
#include "config.h"

/** \brief Set license from null-terminated 'guid' string (SECE only)
 * 
 * The license is saved to a persistent Setting object ("lic" key in
 * LIBENCLOUD_ORG:LIBENCLOUD_APP as defined in defaults.h.
 */
#ifdef LIBENCLOUD_TYPE_SECE
LIBENCLOUD_DLLSPEC libencloud_rc libencloud_setup_set_license (libencloud_t *libencloud, const char *guid)
{
    LIBENCLOUD_RETURN_IF (libencloud == NULL, LIBENCLOUD_RC_BADPARAMS);
    LIBENCLOUD_RETURN_IF (guid == NULL, LIBENCLOUD_RC_BADPARAMS);

    QString s(guid);
    QUuid u(s);

    LIBENCLOUD_RETURN_MSG_IF (u.isNull(), LIBENCLOUD_RC_BADPARAMS, "bad uuid: " << guid);

    LIBENCLOUD_DBG("uiid=" << u << " variant=" << u.variant() << " version=" << u.version());

    libencloud->cfg->settings->setValue("lic", u.toString());
    LIBENCLOUD_RETURN_IF (libencloud->cfg->settings->status(), LIBENCLOUD_RC_SYSERR);

    return LIBENCLOUD_RC_SUCCESS;
}
#endif

/** \brief Get device's serial (LIBENCLOUD only) */
#ifndef LIBENCLOUD_TYPE_SECE
LIBENCLOUD_DLLSPEC const char *libencloud_setup_get_serial (libencloud_t *libencloud)
{
    LIBENCLOUD_RETURN_IF (libencloud == NULL, NULL);

    return libencloud->context->getSerial();
}
#endif

/** \brief Get device's PoI (LIBENCLOUD only) */
#ifndef LIBENCLOUD_TYPE_SECE
LIBENCLOUD_DLLSPEC const char *libencloud_setup_get_poi (libencloud_t *libencloud)
{
    LIBENCLOUD_RETURN_IF (libencloud == NULL, NULL);

    return libencloud->context->getPoi();
}
#endif

