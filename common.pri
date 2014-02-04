# CONFIG options:
#    
#   debug       compile libraries with debugging symbols
#   sece        SECE mode (default is ECE)
#   nosetup     no setup module (e.g. VPN client/manager only)
#   nocloud     no VPN module (e.g. Switchboard setup + external client)

# global defs
PKGNAME = libencloud
# only x.x.x.x format allowed, where x is a number
VERSION = 0.6
VERSION_TAG = Wip  # Dev version - comment this for official release!
QMAKE_TARGET_COMPANY = Endian
QMAKE_TARGET_PRODUCT = libencloud
QMAKE_TARGET_DESCRIPTION = libencloud
QMAKE_TARGET_COPYRIGHT =

QT += core
QT += network
QT -= gui

debug       { QMAKE_CXXFLAGS += -g }
sece        { DEFINES += LIBENCLOUD_TYPE_SECE }
nosetup     { DEFINES += LIBENCLOUD_DISABLE_SETUP }
nocloud     { DEFINES += LIBENCLOUD_DISABLE_CLOUD }

# Platform-specific config and macros
win32 {
    CONFIG += qtjson  # GPL/self-contained

    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += LIBENCLOUD_WIN32

#    system("mkdir $$(APPDATA)\\encloud")
} else {

#   This is OK on Ubuntu but TODO on Yocto
#   CONFIG += qjson   # LGPL/external

    CONFIG += qtjson  # GPL/self-contained
}

DEFINES += LIBENCLOUD_VERSION=\\\"$${VERSION}\\\"
!isEmpty(VERSION_TAG) {
    DEFINES += LIBENCLOUD_VERSION_TAG=\\\"$$VERSION_TAG\\\"
}
exists(".git") {
    DEFINES += LIBENCLOUD_REVISION=\\\"$$system(git rev-parse --short HEAD)\\\"
}
DEFINES += LIBENCLOUD_PKGNAME=\\\"$$PKGNAME\\\"

# uncomment or set globally to avoid debug output
#DEFINES += QT_NO_DEBUG_OUTPUT

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

#
# build settings
# 

SRCBASEDIR = $$PWD

# public API and common headers are included globally
INCLUDEPATH += $$SRCBASEDIR/include
INCLUDEPATH += $$SRCBASEDIR/src/common
DEPENDPATH += $$INCLUDEPATH

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
