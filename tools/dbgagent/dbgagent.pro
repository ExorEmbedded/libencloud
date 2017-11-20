include(../../common.pri)

TEMPLATE = app

# external prog - don't exports dlls
DEFINES -= _LIBENCLOUD_LIB_

CONFIG += console
CONFIG -= app_bundle

TARGET = dbgagent

SOURCES += main.cpp

INCLUDEPATH += $$SRCBASEDIR/src/
DEPENDPATH += $$SRCBASEDIR/src/

# libencloud
LIBS += -L$$OUT_PWD/../../src/$$DESTDIR
LIBS += -lencloud$$DBG_SUFFIX

target.path = $${BINDIR}

INSTALLS += target
