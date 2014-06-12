include(../../common.pri)

TEMPLATE = app

# external prog - don't exports dlls
DEFINES -= _LIBENCLOUD_LIB_

TARGET = unit

CONFIG += gui
CONFIG += console
CONFIG += qtestlib

SOURCES += main.cpp

SOURCES += api.cpp
HEADERS += api.h

SOURCES += crypto.cpp

SOURCES += proxy.cpp
HEADERS += proxy.h

SOURCES += json.cpp
HEADERS += json.h

SOURCES += utils.cpp
HEADERS += utils.h

#SOURCES += libencloud.cpp

HEADERS += test.h

INCLUDEPATH += $$SRCBASEDIR/src/
DEPENDPATH += $$SRCBASEDIR/src/

# libencloud
LIBS += -L$$SRCBASEDIR/src/$$DESTDIR
LIBS += -lencloud$$DBG_SUFFIX

# json - external linkage only for QJson
contains(CONFIG, qjson) {
    LIBS += -lqjson
}

# command to run upon 'make check'
# LIBENCLOUD_WRAP environment variable can be set to "gdb", "valgrind", etc
check.commands = LD_LIBRARY_PATH=:$$SRCBASEDIR/src:$$LIBDIR $$(LIBENCLOUD_WRAP) ./$$TARGET

target.path = $${BINDIR}
INSTALLS += target

unix {
    post.path = $${BINDIR}
    post.extra = $${QMAKE_MOVE} "\"${INSTALL_ROOT}/$${BINDIR}/$${TARGET}\"" "\"${INSTALL_ROOT}/$${BINDIR}/libencloud-test\""
    INSTALLS += post
}
