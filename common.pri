# CONFIG options:
#    
# [ General ]
#
#   endian      Endian build
#   exor        Exor build
#   debug       compile libraries with debugging symbols
#
# [ Modes ]
# 
# Note: mode selection has implications on both behaviour and packaging!
#
#   mode4ic     Endian 4i Connect mode
#   modeece     Endian Cloud Enabler mode
#   modesece    Software Endian Cloud Enabler mode
#
# [ Feature Disabling ]
#
#   nosetup     no setup module (e.g. VPN client/manager only)
#   nocloud     no VPN module (e.g. Switchboard setup + external client)

#
# global defs
#
PKGNAME = libencloud

PRODUCT_4IC="4iConnect"
PRODUCT_JMC="JMConnect"
PRODUCT_ENCLOUD="Encloud"
PRODUCT_SECE="SECE"  # FIXME

# only x.x.x.x format allowed, where x is a number
VERSION = 0.6
VERSION_TAG = Wip  # Dev version - comment this for official release!

endian {
    ORG = Endian
} else:exor {
    ORG = Exor
} else {
    error("organisation must be defined (CONFIG += endian|exor)!")
}

#
# Qt Configuration
#

QT += core
QT += network
QT -= gui

#
# Build config options
#

debug { 
    QMAKE_CXXFLAGS += -g 
}

mode4ic {
    DEFINES += LIBENCLOUD_MODE_4IC
    Endian {
        DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_4IC}\\\"
    } else {
        DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_JMC}\\\"
    }
} else:modeece {
    DEFINES += LIBENCLOUD_MODE_ECE
    DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_ENCLOUD}\\\"
} else:modesece {
    DEFINES += LIBENCLOUD_MODE_SECE
    DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_SECE}\\\"
} else {
    error("a mode must be defined (CONFIG += mode4ic|modeece|modesece)!")
}

nosetup     { DEFINES += LIBENCLOUD_DISABLE_SETUP }
nocloud     { DEFINES += LIBENCLOUD_DISABLE_CLOUD }

# Platform-specific config and macros
win32 {
    CONFIG += qtjson  # GPL/self-contained
    DEFINES += _CRT_SECURE_NO_WARNINGS
} else {
#   This is OK on Ubuntu but TODO on Yocto
#   CONFIG += qjson   # LGPL/external
    CONFIG += qtjson  # GPL/self-contained
}

DEFINES += LIBENCLOUD_VERSION=\\\"$${VERSION}\\\"
!isEmpty(VERSION_TAG) {
    DEFINES += LIBENCLOUD_VERSION_TAG=\\\"$${VERSION_TAG}\\\"
}
exists(".git") {
    DEFINES += LIBENCLOUD_REVISION=\\\"$$system(git rev-parse --short HEAD)\\\"
}
DEFINES += LIBENCLOUD_PKGNAME=\\\"$${PKGNAME}\\\"
DEFINES += LIBENCLOUD_ORG=\\\"$${ORG}\\\"

# uncomment or set globally to avoid debug output
#DEFINES += QT_NO_DEBUG_OUTPUT

# openssl
OPENSSLPATH = $${OPENSSL_INSTALLPATH}
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

SRCBASEDIR = $${PWD}

# public API and headers are included globally
INCLUDEPATH += $${SRCBASEDIR}/include
INCLUDEPATH += $${SRCBASEDIR}/
DEPENDPATH += $${INCLUDEPATH}

# install dirs
windows {  # used only for dev - installer handles positioning on target
           # and runtime paths are defined in src/common/defaults.h
    INSTALLDIR = %ProgramFiles%/encloud
    LIBDIR = $${INSTALLDIR}/lib
    BINDIR = $${INSTALLDIR}/bin
    INCDIR = $${INSTALLDIR}/include
    CONFDIR = %APPDATA%/encloud
} else {  # used for dev and production
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
