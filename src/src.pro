include(../common.pri)

TEMPLATE = lib

TARGET = ece

HEADERS += helpers.h
HEADERS += defaults.h

# ece public header in ../include/
SOURCES += ece.cpp

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
SOURCES += crypto.c

# openssl
LIBS += -lssl -lcrypto

target.path = $$LIBDIR
INSTALLS += target
