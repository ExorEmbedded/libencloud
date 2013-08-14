include(../common.pri)

TEMPLATE = lib

TARGET = ece

HEADERS += helpers.h
HEADERS += defaults.h

SOURCES += ece.cpp

HEADERS += client.h
SOURCES += client.cpp

HEADERS += msg.h
SOURCES += msg.cpp

SOURCES += config.cpp
HEADERS += config.h

HEADERS += json.h
SOURCES += json.cpp

HEADERS += utils.h
SOURCES += utils.cpp

HEADERS += crypto.h
SOURCES += crypto.c

# openssl
LIBS += -lssl -lcrypto

target.path = $$LIBDIR
INSTALLS += target
