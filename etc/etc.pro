include(../common.pri)

TEMPLATE = subdirs

FILES += *.json

# configuration file installation
conf.path = $${CONFDIR}
conf.files = $${FILES}
#INSTALLS += conf
