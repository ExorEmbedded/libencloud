#ifndef _ECE_DEFAULTS_H_
#define _ECE_DEFAULTS_H_

#define ECE_STRING              "libece v"ECE_VERSION

/* Timeout in ms */
#define ECE_SB_URL              "https://ws1.test/"
#define ECE_TIMEOUT             10000
#define ECE_URL_SZ              1024
#define ECE_PATH_SZ             512
#define ECE_PREFIX_PATH         "/tmp/local"          // TODO make configurable (c:/ on win?)
#define ECE_CONF_PATH           "/etc/ece/ece.json"
#define ECE_CACERT_PATH         "/etc/ece/cacert.pem"
#define ECE_CERT1_PATH          "/etc/ece/cert1.pem"
#define ECE_KEY1_PATH           "/etc/ece/key1.pem"
#define ECE_CERT2_PATH          "/etc/ece/cert2.pem"
#define ECE_KEY2_PATH           "/etc/ece/key2.pem"

#define ECE_CMD_GETLICENSE      "command.access.cloud.getLicense"
#define ECE_CMD_GETCSRTEMPLATE  "command.access.cloud.getCSRTemplate"
#define ECE_CMD_SENDCSR         "command.access.cloud.sendCertificateRequest"
#define ECE_CMD_GETCONFIG       "command.access.cloud.getConfiguration"

#endif
