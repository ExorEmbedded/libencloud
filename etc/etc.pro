include(../common.pri)

TEMPLATE = subdirs

modeqic: CONFFILE = libencloud-qic.json
modeece|modesece: CONFFILE = libencloud-ece.json

conf.path = $${CONFDIR}
conf.files += $${CONFFILE}

win32 {
    DDIR = $${CONFDIR}
} else {
    DDIR = ${INSTALL_ROOT}/$${CONFDIR}
}
FROM = $${DDIR}/$${CONFFILE}
TO = $${DDIR}/libencloud.json

win32:FROM ~= s,/,\\,g
win32:TO ~= s,/,\\,g

post.path = $${CONFDIR}
post.extra = $${QMAKE_MOVE} "\"$${FROM}\"" "\"$${TO}\""

INSTALLS += conf
INSTALLS += post
