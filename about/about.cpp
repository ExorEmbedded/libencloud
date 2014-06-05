#include "about.h"

#include <QDebug>
#include <string.h>

static char libEncloudProductDirBuffer [256];
static char libEncloudInstallDirBuffer [512];
static char libEncloudTapNameBuffer [256];

LIBENCLOUDABOUT_DLLSPEC const char* LibEncloudOrganization = BRAND_ORGANIZATION;

// Not used - just make compiler happy
void libEncloudAbout()
{
    Q_UNUSED(libEncloudProductDirBuffer);
    Q_UNUSED(libEncloudInstallDirBuffer);
    Q_UNUSED(libEncloudTapNameBuffer);
}

LIBENCLOUDABOUT_DLLSPEC const char* LibEncloudProductDir(char* buffer, const char* productName)
{
    strncpy(buffer, LibEncloudOrganization, 120);
    strncat(buffer, "/", 1);
    strncat(buffer, productName, 120);
    return buffer;
}

LIBENCLOUDABOUT_DLLSPEC const char* LibEncloudInstallDir(char* buffer, const char* productName, const char* appName)
{
    strncpy(buffer, LibEncloudProductDir(buffer, productName), 250);
    strncat(buffer, "/", 1);
    strncat(buffer, appName, 250);
    return buffer;
}

LIBENCLOUDABOUT_DLLSPEC const char* LibEncloudFullAppName(char* buffer, const char* pkgName)
{
    strncpy(buffer, LibEncloudOrganization, 120);
    strncat(buffer, " ", 1);
    strncat(buffer, pkgName, 120);
    return buffer;
}

LIBENCLOUDABOUT_DLLSPEC const char* LibEncloudTapName(char* buffer, const char* productName)
{
    strncpy(buffer, LibEncloudOrganization, 120);
    strncat(buffer, productName, 120);
    return buffer;
}
