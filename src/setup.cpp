#include <encloud/setup.h>
#include "core.h"
#include "common.h"
#include "config.h"

/** \brief Set license from null-terminated 'guid' string (SECE only)
 * 
 * The license is saved to a persistent Setting object ("lic" key in
 * ENCLOUD_ORG:ENCLOUD_APP as defined in defaults.h.
 */
#ifdef ENCLOUD_TYPE_SECE
ENCLOUD_DLLSPEC encloud_rc encloud_setup_set_license (encloud_t *encloud, const char *guid)
{
    ENCLOUD_RETURN_IF (encloud == NULL, ENCLOUD_RC_BADPARAMS);
    ENCLOUD_RETURN_IF (guid == NULL, ENCLOUD_RC_BADPARAMS);

    QString s(guid);
    QUuid u(s);

    ENCLOUD_RETURN_MSG_IF (u.isNull(), ENCLOUD_RC_BADPARAMS, "bad uuid: " << guid);

    ENCLOUD_DBG("uiid=" << u << " variant=" << u.variant() << " version=" << u.version());

    encloud->cfg->settings->setValue("lic", u.toString());
    ENCLOUD_RETURN_IF (encloud->cfg->settings->status(), ENCLOUD_RC_SYSERR);

    return ENCLOUD_RC_SUCCESS;
}
#endif

/** \brief Get device's serial (ENCLOUD only) */
#ifndef ENCLOUD_TYPE_SECE
ENCLOUD_DLLSPEC const char *encloud_setup_get_serial (encloud_t *encloud)
{
    ENCLOUD_RETURN_IF (encloud == NULL, NULL);

    return encloud->context->getSerial();
}
#endif

/** \brief Get device's PoI (ENCLOUD only) */
#ifndef ENCLOUD_TYPE_SECE
ENCLOUD_DLLSPEC const char *encloud_setup_get_poi (encloud_t *encloud)
{
    ENCLOUD_RETURN_IF (encloud == NULL, NULL);

    return encloud->context->getPoi();
}
#endif

