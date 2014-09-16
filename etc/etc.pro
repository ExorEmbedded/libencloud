include(../common.pri)

TEMPLATE = subdirs

modeqcc: CONFFILE = libencloud-qcc.json
modeece|modesece: CONFFILE = libencloud-ece.json

win32 {
    DDIR = $${CONFDIR}
} else {
    DDIR = ${INSTALL_ROOT}/$${CONFDIR}
}
FROM = $${CONFDIR}/$${CONFFILE}
TO = $${DDIR}/libencloud.json

win32:FROM ~= s,/,\\,g
win32:TO ~= s,/,\\,g

post.path = $${CONFDIR}
post.extra = $${QMAKE_COPY} "\"$${FROM}\"" "\"$${TO}\""

INSTALLS += post
