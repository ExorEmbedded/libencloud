include(../common.pri)

TEMPLATE = app

TARGET = test

SOURCES += main.cpp
SOURCES += crypto.cpp
SOURCES += ece.cpp

INCLUDEPATH += ../src

# openssl
LIBS += -lssl -lcrypto

# libece
LIBS += -L../src -lece

check.commands = LD_LIBRARY_PATH=../src ./test
