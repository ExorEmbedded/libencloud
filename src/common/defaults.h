#ifndef _LIBENCLOUD_PRIV_DEFAULTS_H_
#define _LIBENCLOUD_PRIV_DEFAULTS_H_

/* This file should NOT be Qt-specific (used by pure C modules) */

#define LIBENCLOUD_ORG                 "Endian"
#define LIBENCLOUD_APP                 "libencloud"
#define LIBENCLOUD_PRODUCT             LIBENCLOUD_ORG" "LIBENCLOUD_APP

#ifndef LIBENCLOUD_REVISION
#define LIBENCLOUD_REVISION            ""
#define LIBENCLOUD_STRING              LIBENCLOUD_APP" v"LIBENCLOUD_VERSION
#else
#define LIBENCLOUD_STRING              LIBENCLOUD_APP" v"LIBENCLOUD_VERSION" r"LIBENCLOUD_REVISION
#endif

#define LIBENCLOUD_SB_URL              "https://sb-host/"
#define LIBENCLOUD_TIMEOUT             10000  /* ms */
#define LIBENCLOUD_URL_SZ              1024
#define LIBENCLOUD_PATH_SZ             512
#define LIBENCLOUD_DESC_SZ             64
#ifndef LIBENCLOUD_PREFIX_PATH                             /* [overridable] */
#define LIBENCLOUD_PREFIX_PATH         "/"
#endif
#ifdef _WIN32
#define LIBENCLOUD_ETC_PREFIX          "\\etc\\encloud\\"
#else
#define LIBENCLOUD_ETC_PREFIX          "/etc/encloud/"
#endif
#define LIBENCLOUD_CONF_PATH           LIBENCLOUD_ETC_PREFIX"libencloud.json"
#define LIBENCLOUD_SERIAL_PATH         LIBENCLOUD_ETC_PREFIX"serial"
#define LIBENCLOUD_POI_PATH            LIBENCLOUD_ETC_PREFIX"poi"
#define LIBENCLOUD_CSRTMPL_PATH        LIBENCLOUD_ETC_PREFIX"csr-tmpl.json"
#define LIBENCLOUD_INIT_CA_PATH        LIBENCLOUD_ETC_PREFIX"init_ca.pem"
#define LIBENCLOUD_INIT_CERT_PATH      LIBENCLOUD_ETC_PREFIX"init_cert.pem"
#define LIBENCLOUD_INIT_KEY_PATH       LIBENCLOUD_ETC_PREFIX"init_key.pem"
#define LIBENCLOUD_OP_CERT_PATH        LIBENCLOUD_ETC_PREFIX"op_cert.pem"
#define LIBENCLOUD_OP_KEY_PATH         LIBENCLOUD_ETC_PREFIX"op_key.pem"
#define LIBENCLOUD_RSA_BITS            1024

#define LIBENCLOUD_CMD_GETINFO         "manage/commands/commands.access.cloud.getInfo"
#define LIBENCLOUD_CMD_GETCERT         "manage/commands/commands.access.cloud.getCertificate"
#define LIBENCLOUD_CMD_GETCONFIG       "manage/commands/commands.access.cloud.getConfiguration"

#define LIBENCLOUD_GETCONFIG_HOSTNAME  "operation-host"

#define LIBENCLOUD_LOG_LEV             3

#endif  /* _LIBENCLOUD_PRIV_CRYPTO_H_ */
