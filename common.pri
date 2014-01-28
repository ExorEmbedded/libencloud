# CONFIG options:
#    
#   debug       compile libraries with debugging symbols
#   sece        SECE mode (default is ECE)
#   nosetup     no setup module (e.g. VPN client/manager only)
#   nocloud     no VPN module (e.g. Switchboard setup + external client)

# global defs
PKGNAME = libencloud
# only x.x.x.x format allowed, where x is a number
VERSION = 0.5
QMAKE_TARGET_COMPANY = Endian
QMAKE_TARGET_PRODUCT = libencloud
QMAKE_TARGET_DESCRIPTION = libencloud
QMAKE_TARGET_COPYRIGHT =

QT += core
QT += network
QT -= gui

debug       { QMAKE_CXXFLAGS += -g }
sece        { DEFINES += ENCLOUD_TYPE_SECE }
nosetup     { DEFINES += ENCLOUD_DISABLE_SETUP }
nocloud     { DEFINES += ENCLOUD_DISABLE_CLOUD }

# Platform-specific config and macros
win32 {
    CONFIG += qtjson  # GPL/self-contained

    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += ENCLOUD_WIN32

#    system("mkdir $$(APPDATA)\\encloud")
} else {

#   This is OK on Ubuntu but TODO on Yocto
#   CONFIG += qjson   # LGPL/external

    CONFIG += qtjson  # GPL/self-contained
}

DEFINES += ENCLOUD_VERSION=\\\"$${VERSION}\\\"
exists(".git") {
    DEFINES += ENCLOUD_REVISION=\\\"$$system(git rev-parse --short HEAD)\\\"
}

# uncomment or set globally to avoid debug output
#DEFINES += QT_NO_DEBUG_OUTPUT

# *old-style* static/compile-time logging; log level can be changed dynamically
# by changing value of "log/lev" in /etc/encloud/encloud.json
#DEFINES += ENCLOUD_LOGLEVEL=7

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
    INSTALLDIR = %ProgramFiles%/encloud
    LIBDIR = $${INSTALLDIR}/lib
    BINDIR = $${INSTALLDIR}/bin
    INCDIR = $${INSTALLDIR}/include
    CONFDIR = %APPDATA%/encloud
} else {
    INSTALLDIR = /usr/local
    LIBDIR = $${INSTALLDIR}/lib
    BINDIR = $${INSTALLDIR}/bin
    INCDIR = $${INSTALLDIR}/include
    CONFDIR = /etc/encloud
}

# keep build files separate from sources
UI_DIR = build/uics
MOC_DIR = build/mocs
OBJECTS_DIR = build/objs
Release:DESTDIR = release
Debug:DESTDIR = debug
