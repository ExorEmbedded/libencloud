#ifndef _LIBENCLOUD_PRIV_DEFAULTS_H_
#define _LIBENCLOUD_PRIV_DEFAULTS_H_

#include <encloud/Common>

#define LIBENCLOUD_APP                  LIBENCLOUD_PKGNAME
#define LIBENCLOUD_APP_FULL             LIBENCLOUD_ORG " " LIBENCLOUD_PKGNAME

// e.g Endian/4iConnect, Exor/JMConnect
#define LIBENCLOUD_PRODUCTDIR           LIBENCLOUD_ORG "/" LIBENCLOUD_PRODUCT

// e.g Endian/4iConnect/libencloud | Exor/JMConnect/libencloud
#define LIBENCLOUD_INSTALLDIR           LIBENCLOUD_PRODUCTDIR "/" LIBENCLOUD_APP

#ifndef LIBENCLOUD_REVISION
#  define LIBENCLOUD_REVISION           ""
#  define LIBENCLOUD_STRING             LIBENCLOUD_APP " v" LIBENCLOUD_VERSION
#else
#  define LIBENCLOUD_STRING             LIBENCLOUD_APP " v" LIBENCLOUD_VERSION " rev" LIBENCLOUD_REVISION
#endif

// tap device - MUST match name defined in CheckTap
#define LIBENCLOUD_TAPNAME              LIBENCLOUD_ORG LIBENCLOUD_PRODUCT

#define LIBENCLOUD_USERAGENT            LIBENCLOUD_STRING

// do NOT change this - Switchboard relies on it!
#define LIBENCLOUD_USERAGENT_QIC        "Endian 4i Connect 3.0"

#define LIBENCLOUD_SB_URL               "https://switchboard-host/"

// timeout without exponential backoff
#define LIBENCLOUD_RETRY_TIMEOUT        5

// exponent for timeout with exponential backoff - base increases by 1 at each failure
#define LIBENCLOUD_RETRY_BASE           2

#define LIBENCLOUD_TIMEOUT              10000  /* ms */
#define LIBENCLOUD_URL_SZ               1024
#define LIBENCLOUD_PATH_SZ              512
#define LIBENCLOUD_DESC_SZ              64
#ifndef LIBENCLOUD_PREFIX_PATH                             /* [overridable] */
#define LIBENCLOUD_PREFIX_PATH          "/"
#endif
#ifdef Q_OS_WIN32  // relative paths - refer to src/common/config.cpp
#  define LIBENCLOUD_EXE                ".exe"
#  define LIBENCLOUD_ETC_PREFIX         "\\etc\\"   // => %ProgramFiles% \ LIBENCLOUD_INSTALLDIR \ etc
#  define LIBENCLOUD_SBIN_PREFIX        "\\bin\\"  // => %ProgramFiles% \ LIBENCLOUD_PRODUCTDIR \ bin
#  define LIBENCLOUD_DATA_PREFIX        ""          // => %AppData% \ LIBENCLOUD_INSTALLDIR
#else  // unix - absolute paths
#  define LIBENCLOUD_EXE                ""
#  define LIBENCLOUD_ETC_PREFIX         "/etc/encloud/"
#  define LIBENCLOUD_SBIN_PREFIX        "/usr/sbin/"
#  define LIBENCLOUD_DATA_PREFIX        "/var/lib/encloud/"
#endif
#define LIBENCLOUD_CONF_FILE            LIBENCLOUD_APP ".json"
#define LIBENCLOUD_SERIAL_FILE          "serial"
#define LIBENCLOUD_POI_FILE             "poi"
#define LIBENCLOUD_CSRTMPL_FILE         "csr-tmpl.json"
#define LIBENCLOUD_INIT_CA_FILE         "init_ca.pem"
#define LIBENCLOUD_INIT_CERT_FILE       "init_cert.pem"
#define LIBENCLOUD_INIT_KEY_FILE        "init_key.pem"
#define LIBENCLOUD_OP_CERT_FILE         "op_cert.pem"
#define LIBENCLOUD_OP_KEY_FILE          "op_key.pem"
#define LIBENCLOUD_VPN_EXE_FILE         "openvpn" LIBENCLOUD_EXE
#define LIBENCLOUD_VPN_CONF_FILE        "openvpn.conf"
#define LIBENCLOUD_VPN_MGMT_HOST        LIBENCLOUD_LOCALHOST
#define LIBENCLOUD_VPN_MGMT_PORT        1195
#define LIBENCLOUD_VPN_VERBOSITY        2
#define LIBENCLOUD_RSA_BITS             1024

#define LIBENCLOUD_CMD_GETINFO          "manage/commands/commands.access.cloud.getInfo"
#define LIBENCLOUD_CMD_GETCERT          "manage/commands/commands.access.cloud.getCertificate"
#define LIBENCLOUD_CMD_GETCONFIG        "manage/commands/commands.access.cloud.getConfiguration"

#define LIBENCLOUD_LOG_LEV              3

#endif  /* _LIBENCLOUD_PRIV_CRYPTO_H_ */
