include(../common.pri)

TEMPLATE = lib

TARGET = encloud
win32 {
    CONFIG += dll
    # disable number in mingw output (libencloud0.dll vs libencloud.dll)
    TARGET_EXT = .dll
}

# define used to select dllimport/dllexport attributes
DEFINES += _LIBENCLOUDLIB_

HEADERS += common.h
HEADERS += defaults.h
HEADERS += helpers.h

SOURCES += core.cpp
HEADERS += core.h

SOURCES += context.cpp
HEADERS += context.h

SOURCES += thread.cpp
HEADERS += thread.h

SOURCES += setup.cpp

HEADERS += vpn.h
SOURCES += vpn.cpp

HEADERS += client.h
SOURCES += client.cpp

HEADERS += msg.h
SOURCES += msg.cpp

HEADERS += config.h
SOURCES += config.cpp

HEADERS += json.h
SOURCES += json.cpp

# old GPL Json implementation (small: self-contained)
contains(CONFIG, qtjson) {
    HEADERS += qtjson.h
    SOURCES += qtjson.cpp
    SOURCES += json-qtjson.cpp
}

# new/default LGPL Json implementation (larger: external package)
contains(CONFIG, qjson) {
    SOURCES += json-qjson.cpp
    LIBS += -lqjson
}

HEADERS += utils.h
SOURCES += utils.cpp

HEADERS += crypto.h
SOURCES += crypto.cpp

target.path = $$LIBDIR
INSTALLS += target
