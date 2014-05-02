include(../../common.pri)

TEMPLATE = app

CONFIG += gui
CONFIG += console
CONFIG += qtestlib

TARGET = unit

SOURCES += main.cpp

SOURCES += api.cpp
HEADERS += api.h

SOURCES += proxy.cpp
HEADERS += proxy.h

SOURCES += json.cpp
HEADERS += json.h

SOURCES += crypto.cpp

#SOURCES += libencloud.cpp

HEADERS += test.h

windows {
    # this is required in order to use bio functions of openssl dll
    SOURCES += $$OPENSSLPATH\\include\\openssl\\applink.c
}

INCLUDEPATH += $$SRCBASEDIR/src/
DEPENDPATH += $$SRCBASEDIR/src/

# libencloud
win32 {
    *-g++* {
        # MinGW
        LIBS += $$SRCBASEDIR/src/$$DESTDIR/libencloud0.a
    }
    *-msvc* {
        # MSVC
        LIBS += $$SRCBASEDIR/src/$$DESTDIR/encloud.lib
    }
} else {
    LIBS += -L$$SRCBASEDIR/src/ -lencloud
}

# json - external linkage only for QJson
contains(CONFIG, qjson) {
    LIBS += -lqjson
}

# command to run upon 'make check'
# LIBENCLOUD_WRAP environment variable can be set to "gdb", "valgrind", etc
check.commands = LD_LIBRARY_PATH=:$$SRCBASEDIR/src:$$LIBDIR $$(LIBENCLOUD_WRAP) ./$$TARGET

unix {
    target.path = $${BINDIR}
    post.path = $${BINDIR}
    post.extra = $${CMD_MV} ${INSTALL_ROOT}/$${BINDIR}/$${TARGET} ${INSTALL_ROOT}/$${BINDIR}/libencloud-test
    INSTALLS += target
    INSTALLS += post
}
