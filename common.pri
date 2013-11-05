# global defs
PKGNAME = libece
# only x.x.x.x format allowed, where x is a number
VERSION = 0.5
QMAKE_TARGET_COMPANY = Endian
QMAKE_TARGET_PRODUCT = libECE
QMAKE_TARGET_DESCRIPTION = libECE
QMAKE_TARGET_COPYRIGHT =

QT += core
QT += network
QT -= gui

# debugging: pass it from command line: qmake CONFIG+=debug
# CONFIG += debug

# json
win32 {
    CONFIG += qtjson  # GPL/self-contained
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += ECE_WIN32
    # working dir is test, etc is one level up
    DEFINES += ECE_PREFIX_PATH=\\\"..\\\"
    system("mkdir $$(APPDATA)\\sece")
} else {

#   This is OK on Ubuntu but TODO on Yocto
#   CONFIG += qjson   # LGPL/external

    CONFIG += qtjson  # GPL/self-contained
}

DEFINES += ECE_VERSION=\\\"$${VERSION}\\\"
exists(".git") {
    DEFINES += ECE_REVISION=\\\"$$system(git rev-parse --short HEAD)\\\"
}

# uncomment or set globally to avoid debug output
#DEFINES += QT_NO_DEBUG_OUTPUT

# *old-style* static/compile-time logging; log level can be changed dynamically
# by changing value of "log/lev" in /etc/ece/ece.json
#DEFINES += ECE_LOGLEVEL=7

# public API is included globally
INCLUDEPATH += ../include

# openssl
OPENSSLPATH = $$(OPENSSL_INSTALLPATH)
windows{
    isEmpty(OPENSSLPATH){
        OPENSSLPATH="c:\\openssl"
    }
    INCLUDEPATH += "$$OPENSSLPATH\\include\\"
    LIBS += $$OPENSSLPATH\\lib\\libeay32.lib
} else {
    LIBS += -lssl -lcrypto
}

# install dirs
windows {
    INSTALLDIR = %ProgramFiles%/SECE
    LIBDIR = $${INSTALLDIR}/lib
    BINDIR = $${INSTALLDIR}/bin
    INCDIR = $${INSTALLDIR}/include
    CONFDIR = %APPDATA%/sece
} else {
    INSTALLDIR = /usr/local
    LIBDIR = $${INSTALLDIR}/lib
    BINDIR = $${INSTALLDIR}/bin
    INCDIR = $${INSTALLDIR}/include
    CONFDIR = /etc/ece
}

# keep build files separate from sources
UI_DIR = build/uics
MOC_DIR = build/mocs
OBJECTS_DIR = build/objs
Release:DESTDIR = release
Debug:DESTDIR = debug
