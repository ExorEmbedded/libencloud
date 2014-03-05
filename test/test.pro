include(../common.pri)

TEMPLATE = app

CONFIG += gui
CONFIG += console
CONFIG += qtestlib

TARGET = test

SOURCES += main.cpp

SOURCES += api.cpp
HEADERS += api.h

SOURCES += json.cpp
HEADERS += json.h

SOURCES += crypto.cpp

#SOURCES += libencloud.cpp

HEADERS += test.h

windows {
    # this is required in order to use bio functions of openssl dll
    SOURCES += $$OPENSSLPATH\\include\\openssl\\applink.c
}

INCLUDEPATH += ../src/
DEPENDPATH += ../src/

# libencloud
win32 {
    *-g++* {
        # MinGW
        LIBS += ../src/$$DESTDIR/libencloud0.a
    }
    *-msvc* {
        # MSVC
        LIBS += ../src/$$DESTDIR/encloud.lib
    }
} else {
    LIBS += -L../src/ -lencloud
}

# json - external linkage only for QJson
contains(CONFIG, qjson) {
    LIBS += -lqjson
}

# command to run upon 'make check'
# LIBENCLOUD_WRAP environment variable can be set to "gdb", "valgrind", etc
check.commands = LD_LIBRARY_PATH=:../src:$$LIBDIR $$(LIBENCLOUD_WRAP) ./test
