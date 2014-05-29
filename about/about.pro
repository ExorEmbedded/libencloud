include(../common.pri)

TEMPLATE = lib

win32 {
    CONFIG += dll
    # disable number in mingw output (libencloud0.dll vs libencloud.dll)
    TARGET_EXT = .dll
} else {
    CONFIG += shared
}

CONFIG -= gui
CONFIG += console

TARGET = about$$DBG_SUFFIX
DESTDIR = $$DESTDIR/$$BRAND

warning($$DESTDIR)

isEmpty(BRAND) {
# use the default main ORG (exor or endian) as brand
BRAND = $$ORG
}

DEFINES += BRAND_ORGANIZATION=\\\"$$BRAND\\\"
DEFINES +=_LIBENCLOUDABOUT_LIB_

SOURCES += about.cpp
HEADERS += about.h


# command to run upon 'make check'
# LIBENCLOUD_WRAP environment variable can be set to "gdb", "valgrind", etc
check.commands = LD_LIBRARY_PATH=:$$SRCBASEDIR/src:$$LIBDIR $$(LIBENCLOUD_WRAP) ./$$TARGET

target.path = $${BINDIR}
INSTALLS += target

unix {
    post.path = $${BINDIR}
    post.extra = $${QMAKE_MOVE} "\"${INSTALL_ROOT}/$${BINDIR}/$${TARGET}\"" "\"${INSTALL_ROOT}/$${BINDIR}/about.dll\""
    INSTALLS += post
}

