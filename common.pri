# global defs
PKGNAME = libece
VERSION = 0.1

QT += core
QT += network
QT -= gui

# debugging
CONFIG += debug

DEFINES += ECE_VERSION=\\\"$$VERSION\\\"

# uncomment or set globally to avoid debug output
#DEFINES += QT_NO_DEBUG_OUTPUT

# logging: valid only if QT_NO_DEBUG_OUTPUT is NOT set
# (see README for info)
DEFINES += ECE_LOGLEVEL=3

# public API is included globally
INCLUDEPATH += ../include

# install dirs
INSTALLDIR = /tmp/local
LIBDIR = $$INSTALLDIR/lib
BINDIR = $$INSTALLDIR/bin
INCDIR = $$INSTALLDIR/include
CONFDIR = $$INSTALLDIR/etc/ece

## keep build files separate from sources
# UI_DIR = build/uics
# MOC_DIR = build/mocs
# OBJECTS_DIR = build/objs
