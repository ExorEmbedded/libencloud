include(../common.pri)

TEMPLATE = subdirs

FILES += *.json
FILES += *.pem

# configuration file installation
conf.path = $${CONFDIR}
conf.files = $${FILES}
INSTALLS += conf
