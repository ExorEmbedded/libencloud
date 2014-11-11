include(../common.pri)

TEMPLATE = lib

win32 {
    !nodll {
        CONFIG += dll
        # disable number in mingw output (libencloud0.dll vs libencloud.dll)
        TARGET_EXT = .dll
    } else {
        CONFIG += staticlib
    }
} else {
    CONFIG += shared
}

CONFIG -= gui
CONFIG += console

TARGET = about$$DBG_SUFFIX

isEmpty(BRAND) {
    # use the default main ORG (exor or endian) as brand
    BRAND = $$ORG
}

DEFINES += BRAND_ORGANIZATION=\\\"$$BRAND\\\"
DEFINES +=_LIBENCLOUDABOUT_LIB_

SOURCES += about.cpp
HEADERS += about.h

win32 {
    target.path = $${BINDIR}
} else {
    target.path = $${LIBDIR}
}
INSTALLS += target
