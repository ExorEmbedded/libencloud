include(../common.pri)

TEMPLATE = app

TARGET = test

SOURCES += main.cpp
SOURCES += json.cpp
SOURCES += crypto.cpp
SOURCES += ece.cpp

INCLUDEPATH += ../src

# openssl
LIBS += -lssl -lcrypto

# libece
LIBS += -L../src -lece

# json - external linkage only for QJson
contains(CONFIG, qjson) {
    LIBS += -lqjson
}

# command to run upon 'make check'
check.commands = LD_LIBRARY_PATH=:../src:$$LIBDIR ./test
