include(../common.pri)

TEMPLATE = subdirs

modeqcc: CONFFILE = libencloud-qcc.json

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

certs.files = ca-certificates.crt
certs.path = $${CONFDIR}

INSTALLS += conf
INSTALLS += certs
