include(../common.pri)

TEMPLATE = lib

TARGET = ece

SOURCES += ece.cpp
SOURCES += client.cpp
SOURCES += config.cpp
SOURCES += json.cpp

HEADERS += helpers.h
HEADERS += defaults.h
HEADERS += client.h
HEADERS += config.h
HEADERS += json.h

target.path = $$LIBDIR
INSTALLS += target
