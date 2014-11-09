# CONFIG options:
#    
# [ General ]
#
#   endian      Endian build
#   exor        Exor build
#   debug       compile libraries with debugging symbols
#   nodll       build a static library
#
# [ Modes ]
# 
# Note: mode selection has implications on both behaviour and packaging!
#
#   modeqcc     Endian ConnectClient / Exor JMConnect mode
#   modeece     Endian Cloud Enabler mode
#   modesece    Software Endian Cloud Enabler mode
#   modevpn     Only vpn manager with static ovpn file configuration
#
# [ Feature Disabling ]
#
#   nosetup     no setup module (e.g. VPN client/manager only)
#   nocloud     no VPN module (e.g. Switchboard setup + external client)
#   noapi       no API module (Qt-based HTTP client for Encloud Service)
#   nogui       totally disable Qt Gui modally (also in tests)
#   notest      don't build tests
#
# [ About definitions ]
#   about       use brand info from about dll, instead of using default for major brand (exor/endian)

contains(DEFINES, WINCE) {
    CONFIG += wince
}

# Local configuration overrides. Sample content:
#     CONFIG += endian
#     CONFIG += modeqcc
LOCALCONFIG=$$(HOME)/.qmake.pri
exists($${LOCALCONFIG}): include($${LOCALCONFIG})
LOCALCONFIG=.qmake.pri
exists($${LOCALCONFIG}): include($${LOCALCONFIG})

#
# global defs
#
PKGNAME = libencloud

PRODUCT_ECC="ConnectClient"
PRODUCT_JMC="HMIConnect"
PRODUCT_ENCLOUD="Encloud"
PRODUCT_SECE="SECE"  # FIXME
PRODUCT_VPN="OVPN"  # FIXME

# only x.x.x.x format allowed, where x is a number
VERSION = 0.7.2
#VERSION_TAG = Wip  # Dev version - comment this for official release!
#VERSION_TAG = Beta  # Dev version - comment this for official release!

endian {
    ORG = Endian
} else:exor {
    ORG = Exor
    DEFINES += LIBENCLOUD_EXOR
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

debug:unix { 
    QMAKE_CXXFLAGS += -g 
}

# Suffix for libraries compiled with debug symbols
win32{
    CONFIG(debug,debug|release) {
        DBG_SUFFIX = d
    }
} else {
    DBG_SUFFIX =
}

modeqcc {
    DEFINES += LIBENCLOUD_MODE_QCC
    endian {
        PROGDIR=$$(ProgramFiles)/$${ORG}/$${PRODUCT_ECC}
        DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_ECC}\\\"
    } else {
        PROGDIR=$$(ProgramFiles)/$${ORG}/$${PRODUCT_JMC}
        DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_JMC}\\\"
    }
} else:modeece {
    PROGDIR=$$(ProgramFiles)/$${ORG}/$${PRODUCT_ENCLOUD}
    DEFINES += LIBENCLOUD_MODE_ECE
    DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_ENCLOUD}\\\"
} else:modesece {
    PROGDIR=$$(ProgramFiles)/$${ORG}/$${PRODUCT_SECE}
    DEFINES += LIBENCLOUD_MODE_SECE
    DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_SECE}\\\"
} else:modevpn {
    PROGDIR=$$(ProgramFiles)/$${ORG}/$${PRODUCT_VPN}
    DEFINES += LIBENCLOUD_MODE_VPN
    DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_VPN}\\\"
} else {
    error("a mode must be defined (CONFIG += modeqcc|modeece|modesece|modevpn)!")
}

nosetup     { DEFINES += LIBENCLOUD_DISABLE_SETUP }
nocloud     { DEFINES += LIBENCLOUD_DISABLE_CLOUD }
noapi       { DEFINES += LIBENCLOUD_DISABLE_API }

about       { DEFINES += LIBENCLOUD_USE_ABOUT }

# Platform-specific config and macros
win32 {
    CONFIG += qtjson  # GPL/self-contained
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += _LIBENCLOUD_LIB_  # for dll exports
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
    REVISION=$$system(git rev-parse --short HEAD)
    DEFINES += LIBENCLOUD_REVISION=\\\"$${REVISION}\\\"
} else {
    DEFINES += LIBENCLOUD_REVISION=\\\"$$cat(.revision)\\\"
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
    !exists($$OPENSSLPATH) {
        error("Missing openssl dependency - expected in $$OPENSSL_PATH") 
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
PUBINCLUDEDIR = $${SRCBASEDIR}/include/encloud

# public API and headers are included globally
INCLUDEPATH += $${SRCBASEDIR}/include
INCLUDEPATH += $${SRCBASEDIR}/src
DEPENDPATH += $${INCLUDEPATH}

# install dirs
windows {  # used only for dev - installer handles positioning on target
           # and runtime paths are defined in src/common/defaults.h
    INSTALLDIR = "$${PROGDIR}"
    LIBDIR = "$${INSTALLDIR}/bin"
    BINDIR = "$${INSTALLDIR}/bin"
    CONFDIR = "$${INSTALLDIR}/$${PKGNAME}/etc"
} else {  # used for dev and production
    INSTALLDIR = /usr
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
