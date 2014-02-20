include(../common.pri)

TEMPLATE = subdirs

FILES += *.json

# on windows relative paths are used
unix {
    # configuration file installation
    conf.path = $${CONFDIR}
    conf.files = $${FILES}
    INSTALLS += conf
}
