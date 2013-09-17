include(../common.pri)

TEMPLATE = app
CONFIG += console
TARGET = test

SOURCES += main.cpp
SOURCES += json.cpp
SOURCES += crypto.cpp
SOURCES += ece.cpp

HEADERS += test.h

INCLUDEPATH += ../src

# libece
win32 {
    *-g++* {
        # MinGW
        LIBS += ../src/$$DESTDIR/libece0.a
    }
    *-msvc* {
        # MSVC
        LIBS += ../src/$$DESTDIR/ece.lib
    }
} else {
    LIBS += -L../src/ -lece
}

# json - external linkage only for QJson
contains(CONFIG, qjson) {
    LIBS += -lqjson
}

# command to run upon 'make check'
check.commands = LD_LIBRARY_PATH=:../src:$$LIBDIR ./test
