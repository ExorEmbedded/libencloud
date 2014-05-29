include(../../common.pri)

TEMPLATE = app

# external prog - don't exports dlls
DEFINES -= _LIBENCLOUD_LIB_

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
LIBS = -L$$SRCBASEDIR/src/$$DESTDIR
LIBS += -lencloud$$DBG_SUFFIX

target.path = $${BINDIR}
INSTALLS += target

# command to run upon 'make check'
# LIBENCLOUD_WRAP environment variable can be set to "gdb", "valgrind", etc
check.commands = LD_LIBRARY_PATH=:$$SRCBASEDIR/src:$$LIBDIR $$(LIBENCLOUD_WRAP) ./$$TARGET
