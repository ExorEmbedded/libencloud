include(../../common.pri)

TEMPLATE = app

# external prog - don't exports dlls
DEFINES -= _LIBENCLOUD_LIB_

CONFIG += console
CONFIG -= app_bundle

TARGET = qurl

SOURCES += main.cpp

SOURCES += client.cpp
HEADERS += client.h

INCLUDEPATH += $$SRCBASEDIR/src/
DEPENDPATH += $$SRCBASEDIR/src/

# libencloud
LIBS += -L$$OUT_PWD/../../src/$$DESTDIR
LIBS += -lencloud$$DBG_SUFFIX

target.path = $${BINDIR}

INSTALLS += target

about {
    LIBS += $${ABOUT_LIBS}
}
