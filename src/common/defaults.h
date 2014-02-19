#ifndef _LIBENCLOUD_PRIV_DEFAULTS_H_
#define _LIBENCLOUD_PRIV_DEFAULTS_H_

/* This file should NOT be Qt-specific (used by pure C modules) */

#define LIBENCLOUD_ORG                  "Endian"
#define LIBENCLOUD_APP                  "libencloud"
#define LIBENCLOUD_PRODUCT              LIBENCLOUD_ORG" "LIBENCLOUD_APP

#ifndef LIBENCLOUD_REVISION
#  define LIBENCLOUD_REVISION           ""
#  define LIBENCLOUD_STRING             LIBENCLOUD_APP" v"LIBENCLOUD_VERSION
#else
#  define LIBENCLOUD_STRING             LIBENCLOUD_APP" v"LIBENCLOUD_VERSION" r"LIBENCLOUD_REVISION
#endif

#define LIBENCLOUD_USERAGENT            LIBENCLOUD_STRING
#define LIBENCLOUD_USERAGENT_4IC        "Endian 4i Connect "LIBENCLOUD_VERSION

#define LIBENCLOUD_SB_URL               "https://sb-host/"

// initial timeout for retry (in seconds) - grows exponentially
#define LIBENCLOUD_RETRY_TIMEOUT        3

#define LIBENCLOUD_TIMEOUT              10000  /* ms */
#define LIBENCLOUD_URL_SZ               1024
#define LIBENCLOUD_PATH_SZ              512
#define LIBENCLOUD_DESC_SZ              64
#ifndef LIBENCLOUD_PREFIX_PATH                             /* [overridable] */
#define LIBENCLOUD_PREFIX_PATH          "/"
#endif
#ifdef _WIN32
#  define LIBENCLOUD_ETC_PREFIX        "\\etc\\"
#  define LIBENCLOUD_SBIN_PREFIX        "\\bin\\"
#else
#  define LIBENCLOUD_ETC_PREFIX         "/etc/encloud/"
#  define LIBENCLOUD_SBIN_PREFIX        "/usr/sbin/"
#endif
#define LIBENCLOUD_CONF_FILE            "libencloud.json"
#define LIBENCLOUD_SERIAL_FILE          "serial"
#define LIBENCLOUD_POI_FILE             "poi"
#define LIBENCLOUD_CSRTMPL_FILE         "csr-tmpl.json"
#define LIBENCLOUD_INIT_CA_FILE         "init_ca.pem"
#define LIBENCLOUD_INIT_CERT_FILE       "init_cert.pem"
#define LIBENCLOUD_INIT_KEY_FILE        "init_key.pem"
#define LIBENCLOUD_OP_CERT_FILE         "op_cert.pem"
#define LIBENCLOUD_OP_KEY_FILE          "op_key.pem"
#define LIBENCLOUD_VPN_EXE_FILE         "openvpn"
#define LIBENCLOUD_VPN_CONF_FILE        "openvpn.conf"
#define LIBENCLOUD_VPN_MGMT_HOST        "localhost"
#define LIBENCLOUD_VPN_MGMT_PORT        1195
#define LIBENCLOUD_VPN_VERBOSITY        2
#define LIBENCLOUD_RSA_BITS             1024

#define LIBENCLOUD_CMD_GETINFO          "manage/commands/commands.access.cloud.getInfo"
#define LIBENCLOUD_CMD_GETCERT          "manage/commands/commands.access.cloud.getCertificate"
#define LIBENCLOUD_CMD_GETCONFIG        "manage/commands/commands.access.cloud.getConfiguration"

#define LIBENCLOUD_GETCONFIG_HOSTNAME   "operation-host"

#define LIBENCLOUD_LOG_LEV              3

#endif  /* _LIBENCLOUD_PRIV_CRYPTO_H_ */
