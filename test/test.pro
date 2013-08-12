include(../common.pri)

TEMPLATE = app

TARGET = test

SOURCES += main.c

LIBS += -L../src -lece

check.commands = LD_LIBRARY_PATH=../src ./test
