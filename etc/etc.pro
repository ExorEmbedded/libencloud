include(../common.pri)

TEMPLATE = subdirs

modeqic: CONFFILE = libencloud-qic.json
modeece: CONFFILE = libencloud-ece.json

conf.path = $${CONFDIR}
conf.files += $${CONFFILE}

post.path = $${CONFDIR}
post.extra = $${CMD_MV} ${INSTALL_ROOT}/$${CONFDIR}/$${CONFFILE} ${INSTALL_ROOT}/$${CONFDIR}/libencloud.json

INSTALLS += conf
INSTALLS += post
