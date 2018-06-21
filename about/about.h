/** __DEPRECATED__ by light branding */

#ifndef _LIBENCLOUD_ABOUT_H_
#define _LIBENCLOUD_ABOUT_H_

#ifndef LIBENCLOUDABOUT_DLLSPEC
#  if defined(WIN32) || defined(WINCE)
#    if defined(_LIBENCLOUDABOUT_LIB_)
#       define LIBENCLOUDABOUT_DLLSPEC __declspec(dllexport)
#    else
#       define LIBENCLOUDABOUT_DLLSPEC __declspec(dllimport)
#    endif
#  else
#    define LIBENCLOUDABOUT_DLLSPEC
#  endif
#endif

LIBENCLOUDABOUT_DLLSPEC extern const char* LibEncloudOrganization;
/*! Takes buffer in input and returns buffer as output */
LIBENCLOUDABOUT_DLLSPEC extern const char* LibEncloudProductDir(char* buffer, const char* productName);
LIBENCLOUDABOUT_DLLSPEC extern const char* LibEncloudInstallDir(char* buffer, const char* productName, const char* appName);
LIBENCLOUDABOUT_DLLSPEC extern const char* LibEncloudFullAppName(char* buffer, const char* pkgName);
LIBENCLOUDABOUT_DLLSPEC extern const char* LibEncloudTapName(char* buffer, const char* productName);

#endif
