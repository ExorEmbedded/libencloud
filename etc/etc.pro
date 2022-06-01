include(../common.pri)

TEMPLATE = subdirs

modeqcc: CONFFILE = $$PWD/libencloud-qcc.json

win32 {
    DDIR = $${CONFDIR}
} else {
    DDIR = ${INSTALL_ROOT}/$${CONFDIR}
}
FROM = $${CONFFILE}
TO = $${DDIR}/libencloud.json

win32:FROM ~= s,/,\\,g
win32:TO ~= s,/,\\,g

conf.path = $${CONFDIR}
conf.extra = $${QMAKE_COPY} "\"$${FROM}\"" "\"$${TO}\""

certs.files = $$PWD/ca-certificates.crt
certs.path = $${CONFDIR}

# configuration file is now optional / no longer installed
#INSTALLS += conf

INSTALLS += certs
