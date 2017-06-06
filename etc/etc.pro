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

post.path = $${CONFDIR}
post.extra = $${QMAKE_COPY} "\"$${FROM}\"" "\"$${TO}\""

INSTALLS += post
