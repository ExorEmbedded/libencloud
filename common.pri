# CONFIG options:
#    
# [ General ]
#
#   endian      Endian build
#   exor        Exor build
#   debug       compile libraries with debugging symbols
#   nodll       build a static library
#   splitdeps   Dependencies are split into different parts of the system
#               (Default behaviour is to package all dependencies together into a self-contained package/bundle
#               Split mode is used when dependencies are handled as separate packages - e.g. Yocto Encloud services)
#
# [ Modes ]
# 
# Note: mode selection has implications on both behaviour and packaging!
#
#   modeqcc  a) Endian Connect App / Exor JMConnect mode client mode
#            b) Agents in "fake connect mode" that authenticate via Status API (unregistered on EN)
#                config.agent = false
#            c) Agents which use autoregistration API based on activation code (registered on EN)
#                config.agent = true
#   modeece     Endian Cloud Enabler mode (SUSPENDED - deleteme?)
#   modesece    Software Endian Cloud Enabler mode (SUSPENDED - deleteme?)
#   modevpn     Only vpn manager with static ovpn file configuration
#               Note: use this mode only if no setup is ever required
#               (other modes now also support vpn-only mode via Config API)
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
SRCBASEDIR = $${PWD}

ENCLOUD_SRC = $$SRCBASEDIR/../encloud
include($${ENCLOUD_SRC}/defines.pri)

# [ client/package version ]
# *** DO NOT CHANGE THIS TO UPDATE LIBENCLOUD VERSION ***
# used for version in User Agent - particularly important in modeqcc for
# Switchboard update checks
modeqcc:!splitdeps {
    DEFINES += PRODUCT_SRC=$$PRODUCT_SRC
    !exists($${PRODUCT_SRC}): error(Could not find main application sources! Expected in $${PRODUCT_SRC})
    include($${PRODUCT_SRC}/version.pri)
} else {
    # TODO grab version from other products or make configurable via json
    VERSION = 0.0.1 # dummy version to make Encloud Server happy with User-Agent
}

DEFINES += PRODUCT_VERSION=\\\"$${VERSION}\\\"

# [ libencloud version ] 
# *** CHANGE THIS TO UPDATE LIBENCLOUD VERSION ***
# only x.x.x.x format allowed, where x is a number
VERSION = 0.12.0
#VERSION_TAG = Wip  # Dev version - comment this for official release!
#VERSION_TAG = Beta  # Dev version - comment this for official release!

DEFINES += LIBENCLOUD_VERSION=\\\"$${VERSION}\\\"
!isEmpty(VERSION_TAG) {
    DEFINES += LIBENCLOUD_VERSION_TAG=\\\"$${VERSION_TAG}\\\"
}

endian {
    DEFINES += QICC_ENDIAN
} else:exor {
    DEFINES += QICC_EXOR
} else:panda {
} else {
    error("an organization must be specified in CONFIG!")
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

# Platform-specific macros and suffixes
win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += _LIBENCLOUD_LIB_  # for dll exports
    CONFIG(debug,debug|release) {
        DBG_SUFFIX = d
    }
} else {
    DBG_SUFFIX =
}

win32 {
    LIBEXT = lib
} else {
    LIBEXT = a
}

# keep build files separate from sources
UI_DIR = build/uics
MOC_DIR = build/mocs
OBJECTS_DIR = build/objs
Release:DESTDIR = release
Debug:DESTDIR = debug

modeqcc {
    DEFINES += LIBENCLOUD_MODE_QCC
} else:modeece {
    DEFINES += LIBENCLOUD_MODE_ECE
} else:modesece {
    DEFINES += LIBENCLOUD_MODE_SECE
} else:modevpn {
    DEFINES += LIBENCLOUD_MODE_VPN
} else {
    error("a mode must be defined (CONFIG += modeqcc|modeece|modesece|modevpn)!")
}
DEFINES += LIBENCLOUD_PRODUCT=\\\"$${PRODUCT_DIR}\\\"

splitdeps {
    DEFINES += LIBENCLOUD_SPLITDEPS
}

nosetup     { DEFINES += LIBENCLOUD_DISABLE_SETUP }
nocloud     { DEFINES += LIBENCLOUD_DISABLE_CLOUD }
noapi       { DEFINES += LIBENCLOUD_DISABLE_API }

about       { DEFINES += LIBENCLOUD_USE_ABOUT }

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

# json
exor { # app is proprietary
    win32 {
        CONFIG += qtjson  # GPL/self-contained - TODO!
    } else {
        CONFIG += qjson   # LGPL/external
    }
} else {
    CONFIG += qtjson  # GPL/self-contained
}

# openssl
OPENSSLPATH = $${OPENSSL_INSTALLPATH}
windows{
    isEmpty(OPENSSLPATH){
        exor {
            OPENSSLPATH="c:\\openssl"
        } else {  # endian, panda
            OPENSSLPATH="$$SRCBASEDIR/../bins-connectapp/openssl"
        }
    }
    !exists($$OPENSSLPATH) {
        error("Missing openssl dependency - expected in $$OPENSSLPATH") 
    }
    INCLUDEPATH += "$$OPENSSLPATH\\include\\"
    LIBS += $$OPENSSLPATH\\lib\\libeay32.lib
} else {
    LIBS += -lssl -lcrypto
}

# yaml
!splitdeps {
    QTYAML_PATH = $${SRCBASEDIR}/../yaml-cpp
    INCLUDEPATH += $${QTYAML_PATH}/include
    LIBS += -L$${QTYAML_PATH}/src/$$DESTDIR 
}
LIBS += -lyaml-cpp

# libabout
about {
    win32 {
        ABOUT_LIBS += $$SRCBASEDIR/about/$$DESTDIR/about$${DBG_SUFFIX}.$${LIBEXT}
    } unix {
        ABOUT_LIBS += -L$$SRCBASEDIR/about/$$DESTDIR/ -labout
    }
}

# for SHGetFolderPath()
win32 {
    !wince {
        LIBS += -lshfolder
    }
}


#
# build settings
# 

PUBINCLUDEDIR = $${SRCBASEDIR}/include/encloud

# public API and headers are included globally
INCLUDEPATH += $${SRCBASEDIR}/include
INCLUDEPATH += $${SRCBASEDIR}/src
DEPENDPATH += $${INCLUDEPATH}

# 
# install dirs
# 
unix {
    INSTALLDIR = /usr
    LIBDIR = $${INSTALLDIR}/lib
    BINDIR = $${INSTALLDIR}/bin
    INCDIR = $${INSTALLDIR}/include
}

# overrides for Connect packaging
CONNECT_DEFINES=$${SRCBASEDIR}/../connectapp/defines.pri
exists($${CONNECT_DEFINES}): include($${CONNECT_DEFINES})

DEFINES += QICC_SETTING_KEY=$$cat($${SETTINGKEYPATH})

# local overrides
windows {
    CONFDIR = "$${INSTALLDIR}/$${PKGNAME}/etc"
} else {
    CONFDIR = /etc/encloud
}
