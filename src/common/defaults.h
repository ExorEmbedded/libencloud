#ifndef _LIBENCLOUD_PRIV_DEFAULTS_H_
#define _LIBENCLOUD_PRIV_DEFAULTS_H_

#include <encloud/Common>

#define LIBENCLOUD_APP                  LIBENCLOUD_PKGNAME

// tap device - MUST match name defined in CheckTap
#define LIBENCLOUD_TAPNAME              LIBENCLOUD_ORG LIBENCLOUD_TAPNAME_SUFFIX

#ifdef QICC_XBRAND
#  define LIBENCLOUD_APP_FULL             LIBENCLOUD_PKGNAME
#else
#  define LIBENCLOUD_APP_FULL             LIBENCLOUD_ORG " " LIBENCLOUD_PKGNAME
#endif

// e.g Endian/ConnectApp, Exor/JMConnect
#define LIBENCLOUD_PRODUCTDIR           LIBENCLOUD_ORG "/" LIBENCLOUD_PRODUCT

// e.g Endian/ConnectApp/libencloud | Exor/JMConnect/libencloud
#define LIBENCLOUD_INSTALLDIR           LIBENCLOUD_PRODUCTDIR "/" LIBENCLOUD_APP

#ifndef LIBENCLOUD_REVISION
#  define LIBENCLOUD_REVISION           ""
#  define LIBENCLOUD_STRING             LIBENCLOUD_APP " v" LIBENCLOUD_VERSION
#else
#  define LIBENCLOUD_STRING             LIBENCLOUD_APP " v" LIBENCLOUD_VERSION " rev" LIBENCLOUD_REVISION
#endif

#ifndef LIBENCLOUD_VERSION_TAG
#  define LIBENCLOUD_VERSION_TAG        ""
#endif

#if defined Q_OS_WIN32
#  define LIBENCLOUD_OS_STR             "Windows"
#elif defined Q_OS_LINUX
#  define LIBENCLOUD_OS_STR             "Linux"
#elif defined Q_OS_MAC
#  define LIBENCLOUD_OS_STR             "Macintosh"
#elif defined Q_OS_WINCE
#  define LIBENCLOUD_OS_STR             "WinCE"
#else
#  define LIBENCLOUD_OS_STR             "Unknown OS"
#endif

#define LIBENCLOUD_USERAGENT            LIBENCLOUD_STRING " (" LIBENCLOUD_OS_STR ")"

// do NOT change this - Switchboard relies on it!
#define LIBENCLOUD_USERAGENT_QCC        "Endian 4i Connect " PRODUCT_VERSION " (" LIBENCLOUD_OS_STR ")"

#define LIBENCLOUD_SCHEME_HTTPS         "https"
#define LIBENCLOUD_REG_URL              LIBENCLOUD_SCHEME_HTTPS "://switchboard-registry/"
#define LIBENCLOUD_REG_CODE_FILE        "activation-code"
#define LIBENCLOUD_REG_CODE_ENC_FILE    "activation-code_enc"
#define LIBENCLOUD_REG_PROV_ENC_FILE    "provisioning_enc"
#define LIBENCLOUD_SB_URL               LIBENCLOUD_SCHEME_HTTPS "://switchboard-host/"

// time for single requests
#define LIBENCLOUD_CLIENT_TIMEOUT       30

// maximum time allowed to connect to Cloud
#define LIBENCLOUD_TIMEOUT              60     /* seconds */

// exponent for timeout with exponential backoff - base increases by 1 at each failure
#define LIBENCLOUD_RETRY_BASE           2
#define LIBENCLOUD_RETRY_MAX            300  // 5 minutes

#define LIBENCLOUD_URL_SZ               1024
#define LIBENCLOUD_PATH_SZ              512
#define LIBENCLOUD_DESC_SZ              64
#ifndef LIBENCLOUD_PREFIX_PATH                             /* [overridable] */
#define LIBENCLOUD_PREFIX_PATH          "/"
#endif

#ifdef Q_OS_WIN
#  define LIBENCLOUD_EXE                ".exe"
#else
#  define LIBENCLOUD_EXE                ""
#endif

#ifdef Q_OS_WIN
#  define LIBENCLOUD_SCRIPTEXT          ".bat"
#else
#  define LIBENCLOUD_SCRIPTEXT          ".sh"
#endif

#ifndef LIBENCLOUD_SPLITDEPS // relative paths - refer to src/common/config.cpp
#  ifdef Q_OS_WIN32
#    define LIBENCLOUD_ETC_PREFIX       LIBENCLOUD_APP "\\etc\\"   // => %ProgramFiles% \ LIBENCLOUD_INSTALLDIR \ etc
#    define LIBENCLOUD_BIN_PREFIX       "\\bin\\"   // => %ProgramFiles% \ LIBENCLOUD_PRODUCTDIR \ bin
#    define LIBENCLOUD_SBIN_PREFIX      LIBENCLOUD_BIN_PREFIX
#    define LIBENCLOUD_DATA_PREFIX      ""          // => %AppData% \ LIBENCLOUD_INSTALLDIR
#    define LIBENCLOUD_LOG_PREFIX       ""          // => %AppData% \ LIBENCLOUD_INSTALLDIR
#  elif defined Q_OS_WINCE  // wince , local paths
#    define LIBENCLOUD_ETC_PREFIX       ""   // => %ProgramFiles% \ LIBENCLOUD_INSTALLDIR \ etc
#    define LIBENCLOUD_BIN_PREFIX       ""   // => %ProgramFiles% \ LIBENCLOUD_PRODUCTDIR \ bin
#    define LIBENCLOUD_SBIN_PREFIX      LIBENCLOUD_BIN_PREFIX
#    define LIBENCLOUD_DATA_PREFIX      ""          // => %AppData% \ LIBENCLOUD_INSTALLDIR
#    define LIBENCLOUD_LOG_PREFIX       ""          // => %AppData% \ LIBENCLOUD_INSTALLDIR
#  else  // linux/mac
#    ifdef Q_OS_MAC
#      define LIBENCLOUD_ETC_PREFIX     "/Resources/" LIBENCLOUD_APP "/etc"
#    else
#      define LIBENCLOUD_ETC_PREFIX     LIBENCLOUD_APP "/etc"
#    endif
#    define LIBENCLOUD_BIN_PREFIX       ""
#    define LIBENCLOUD_SBIN_PREFIX      LIBENCLOUD_BIN_PREFIX
#    ifdef Q_OS_MAC
#      define LIBENCLOUD_DATA_PREFIX    "/Library/Application Support/"
#    else
#      define LIBENCLOUD_DATA_PREFIX    "/var/lib/"
#    endif
#    define LIBENCLOUD_LOG_PREFIX       LIBENCLOUD_DATA_PREFIX
#  endif
#else // LIBENCLOUD_SPLITDEPS - absolute paths - only used on Linux/Yocto for now
#  define LIBENCLOUD_ETC_PREFIX         "/etc/encloud/"
#  define LIBENCLOUD_BIN_PREFIX         "/usr/bin/"
#  define LIBENCLOUD_SBIN_PREFIX        "/usr/sbin/"
#  ifdef QICC_ENDIAN
#    define LIBENCLOUD_DATA_PREFIX      "/var/efw/encloud/"
#  else
#    define LIBENCLOUD_DATA_PREFIX      "/var/lib/encloud/"
#  endif
#  define LIBENCLOUD_LOG_PREFIX         "/var/log/encloud/"
#endif
#define LIBENCLOUD_CONF_FILE            LIBENCLOUD_APP ".json"
#define LIBENCLOUD_SERIAL_FILE          "serial"
#define LIBENCLOUD_POI_FILE             "poi"
#define LIBENCLOUD_AUTH_SB              "sb"
#define LIBENCLOUD_AUTH_PROXY           "proxy"
#define LIBENCLOUD_AUTH_NONE            "none"
#define LIBENCLOUD_AUTH_USERPASS        "user-pass"
#define LIBENCLOUD_AUTH_X509            "x509"
#define LIBENCLOUD_AUTH_X509_USERPASS   "x509-user-pass"  // 2-factor
#define LIBENCLOUD_AUTH_CERTKEY         "cert-key"
#define LIBENCLOUD_AUTH_PKCS12          "pkcs12"
#define LIBENCLOUD_CA_FILE              "ca.pem"
#define LIBENCLOUD_CERT_FILE            "cert.pem"
#define LIBENCLOUD_KEY_FILE             "key.pem"
#define LIBENCLOUD_P12_FILE             "auth.p12"
#define LIBENCLOUD_INIT_CA_FILE         "init_ca.pem"
#define LIBENCLOUD_INIT_CERT_FILE       "init_cert.pem"
#define LIBENCLOUD_INIT_KEY_FILE        "init_key.pem"
#define LIBENCLOUD_INIT_P12_FILE        "init.p12"
#define LIBENCLOUD_OP_CA_FILE           "op_ca.pem"
#define LIBENCLOUD_OP_CERT_FILE         "op_cert.pem"
#define LIBENCLOUD_OP_KEY_FILE          "op_key.pem"
#define LIBENCLOUD_OP_P12_FILE          "op.p12"
#define LIBENCLOUD_VPN_EXE_FILE         "openvpn" LIBENCLOUD_EXE
#define LIBENCLOUD_VPN_CONF_FILE        "openvpn.conf"
#define LIBENCLOUD_VPN_FALLBACK_CONF_FILE       "openvpn-fallback.conf"
#define LIBENCLOUD_VPN_MGMT_HOST        LIBENCLOUD_LOCALHOST
#define LIBENCLOUD_VPN_MGMT_PORT        1195
#define LIBENCLOUD_VPN_MGMT_PERIOD      2000  // (ms)
#define LIBENCLOUD_VPN_VERBOSITY        2
#ifdef Q_OS_WIN
#  define LIBENCLOUD_VPN_LOG_FILE         "openvpn-log.txt"
#else
#  define LIBENCLOUD_VPN_LOG_FILE         "openvpn.log"
#endif

#define LIBENCLOUD_RSA_BITS             1024

#define LIBENCLOUD_CMD_GETINFO          "manage/commands/commands.access.cloud.getInfo"
#define LIBENCLOUD_CMD_GETCERT          "manage/commands/commands.access.cloud.getCertificate"
#define LIBENCLOUD_CMD_GETCONFIG        "manage/commands/commands.access.cloud.getConfiguration"

#define LIBENCLOUD_LOG_LEV              3

#endif  /* _LIBENCLOUD_PRIV_CRYPTO_H_ */
