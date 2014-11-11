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
LIBS += -L$$OUT_PWD/../../src/$$DESTDIR
LIBS += -lencloud$$DBG_SUFFIX

target.path = $${BINDIR}
INSTALLS += target

# command to run upon 'make check'
# LIBENCLOUD_WRAP environment variable can be set to "gdb", "valgrind", etc
check.commands = LD_LIBRARY_PATH=:$$SRCBASEDIR/src:$$LIBDIR $$(LIBENCLOUD_WRAP) ./$$TARGET

nodll {
    DEFINES += LIBENCLOUD_DLLSPEC=
    DEFINES += LIBENCLOUDABOUT_DLLSPEC=
    # for SHGetFolderPath()
    win32 {
        !wince {
            LIBS += -lshfolder
        }
    }

    PRE_TARGETDEPS  += $$OUT_PWD/../../about/$$DESTDIR/about$${DBG_SUFFIX}.lib
    PRE_TARGETDEPS  += $$OUT_PWD/../../src/$$DESTDIR/encloud$${DBG_SUFFIX}.lib

    LIBS += $$PRE_TARGETDEPS

    message($$LIBS)
}

