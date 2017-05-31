#include "about.h"

#include <QDebug>
#include <string.h>

LIBENCLOUDABOUT_DLLSPEC const char* LibEncloudOrganization = BRAND_ORGANIZATION;

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
