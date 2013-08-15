#ifndef _ECE_DEFAULTS_H_
#define _ECE_DEFAULTS_H_

/* This file should NOT be Qt-specific (used by pure C modules) */

#define ECE_STRING              "libece v"ECE_VERSION

/* Timeout in ms */
#define ECE_SB_URL              "https://ws1.test/"
#define ECE_TIMEOUT             10000
#define ECE_URL_SZ              1024
#define ECE_PATH_SZ             512
#define ECE_DESC_SZ             64
#define ECE_PREFIX_PATH         "/"
#define ECE_CONF_PATH           "/etc/ece/ece.json"
#define ECE_CSRTMPL_PATH        "/etc/ece/csr-tmpl.json"
#define ECE_CACERT_PATH         "/etc/ece/cacert.pem"
#define ECE_CERT1_PATH          "/etc/ece/cert1.pem"
#define ECE_KEY1_PATH           "/etc/ece/key1.pem"
#define ECE_CERT2_PATH          "/etc/ece/cert2.pem"
#define ECE_KEY2_PATH           "/etc/ece/key2.pem"
#define ECE_RSA_BITS            1024

#define ECE_CMD_GETINFO         "command.access.cloud.getInfo"
#define ECE_CMD_GETCERT         "command.access.cloud.getCertificate"
#define ECE_CMD_GETCONFIG       "command.access.cloud.getConfiguration"

#define ECE_SETTINGS_ORG        "com.endian"
#define ECE_SETTINGS_APP        "ECE"

#endif
