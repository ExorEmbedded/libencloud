include(../../common.pri)

TEMPLATE = app

QT += gui

TARGET = sece

SOURCES += main.cpp
HEADERS += helpers.h

SOURCES += mainwindow.cpp
HEADERS += mainwindow.h

SOURCES += manager.cpp
HEADERS += manager.h

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

# command to run upon 'make check'
# LIBENCLOUD_WRAP environment variable can be set to "gdb", "valgrind", etc
check.commands = LD_LIBRARY_PATH=:$$SRCBASEDIR/src:$$LIBDIR $$(LIBENCLOUD_WRAP) ./$$TARGET
