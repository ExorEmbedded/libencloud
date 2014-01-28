#ifndef _ENCLOUD_DEFAULTS_H_
#define _ENCLOUD_DEFAULTS_H_

/* This file should NOT be Qt-specific (used by pure C modules) */

#define ENCLOUD_SETTINGS_ORG        "Endian"
#define ENCLOUD_SETTINGS_APP        "libencloud"
#define ENCLOUD_SETTINGS_PRODUCT    ENCLOUD_SETTINGS_ORG" "ENCLOUD_SETTINGS_APP

#ifndef ENCLOUD_REVISION
#define ENCLOUD_REVISION            ""
#define ENCLOUD_STRING              ENCLOUD_SETTINGS_APP" v"ENCLOUD_VERSION
#else
#define ENCLOUD_STRING              ENCLOUD_SETTINGS_APP" v"ENCLOUD_VERSION" r"ENCLOUD_REVISION
#endif

#define ENCLOUD_SB_URL              "https://sb-host/"
#define ENCLOUD_TIMEOUT             10000  /* ms */
#define ENCLOUD_URL_SZ              1024
#define ENCLOUD_PATH_SZ             512
#define ENCLOUD_DESC_SZ             64
#ifndef ENCLOUD_PREFIX_PATH                             /* [overridable] */
#define ENCLOUD_PREFIX_PATH         "/"
#endif
#ifdef _WIN32
#define ENCLOUD_ETC_PREFIX          "\\etc\\encloud\\"
#else
#define ENCLOUD_ETC_PREFIX          "/etc/encloud/"
#endif
#define ENCLOUD_CONF_PATH           ENCLOUD_ETC_PREFIX"encloud.json"
#define ENCLOUD_SERIAL_PATH         ENCLOUD_ETC_PREFIX"serial"
#define ENCLOUD_POI_PATH            ENCLOUD_ETC_PREFIX"poi"
#define ENCLOUD_CSRTMPL_PATH        ENCLOUD_ETC_PREFIX"csr-tmpl.json"
#define ENCLOUD_INIT_CA_PATH        ENCLOUD_ETC_PREFIX"init_ca.pem"
#define ENCLOUD_INIT_CERT_PATH      ENCLOUD_ETC_PREFIX"init_cert.pem"
#define ENCLOUD_INIT_KEY_PATH       ENCLOUD_ETC_PREFIX"init_key.pem"
#define ENCLOUD_OP_CERT_PATH        ENCLOUD_ETC_PREFIX"op_cert.pem"
#define ENCLOUD_OP_KEY_PATH         ENCLOUD_ETC_PREFIX"op_key.pem"
#define ENCLOUD_RSA_BITS            1024

#define ENCLOUD_CMD_GETINFO         "manage/commands/commands.access.cloud.getInfo"
#define ENCLOUD_CMD_GETCERT         "manage/commands/commands.access.cloud.getCertificate"
#define ENCLOUD_CMD_GETCONFIG       "manage/commands/commands.access.cloud.getConfiguration"

#define ENCLOUD_GETCONFIG_HOSTNAME  "operation-host"

#define ENCLOUD_LOG_LEV             3

#endif
