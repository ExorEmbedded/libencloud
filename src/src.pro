include(../common.pri)

TEMPLATE = lib

TARGET = encloud

win32 {
    !nodll {
        CONFIG += dll
        # disable number in mingw output (libencloud0.dll vs libencloud.dll)
        TARGET_EXT = .dll
    } else {
        CONFIG += staticlib
        DEFINES += LIBENCLOUDABOUT_DLLSPEC=
    }

    TARGET = encloud$${DBG_SUFFIX}
} else {
    CONFIG += shared
}

# touch a version.h file to allow proper version update in about fields
modeqcc:!splitdeps {
    PHONY_DEPS = $${PRODUCT_SRC}/version.pri
    headergen.input = PHONY_DEPS
    headergen.output = $${_PRO_FILE_PWD_}/common/qcc_version.h
    headergen.variable_out = HEADERS
    unix { # needs escaping
        headergen.commands =  echo "/\\* $$_DATE_ : FAKE HEADER USED TO TRIGGER REBUILD OF INCLUDER FILE \\*/" > $${_PRO_FILE_PWD_}/common/qcc_version.h
    } else { # dislikes escaping
        headergen.commands =  echo "/* $$_DATE_ : FAKE HEADER USED TO TRIGGER REBUILD OF INCLUDER FILE */" > $${_PRO_FILE_PWD_}/common/qcc_version.h
    }
    headergen.name = CREATE $${_PRO_FILE_PWD_}/common/qcc_version.h
    headergen.CONFIG += combine no_link
    QMAKE_EXTRA_COMPILERS += headergen

    HEADERS += $${_PRO_FILE_PWD_}/common/qcc_version.h
}

#
# common sources
# 

HEADERS += common/defaults.h
HEADERS += common/helpers.h

HEADERS += common/common.h
SOURCES += common/common.cpp

HEADERS += $${PUBINCLUDEDIR}/Auth
SOURCES += common/auth.cpp

HEADERS += $${PUBINCLUDEDIR}/Client
SOURCES += common/client.cpp

HEADERS += $${PUBINCLUDEDIR}/CookieJar
SOURCES += common/cookiejar.cpp

HEADERS += $${PUBINCLUDEDIR}/Error
SOURCES += common/error.cpp

HEADERS += $${PUBINCLUDEDIR}/Info
SOURCES += common/info.cpp

HEADERS += $${PUBINCLUDEDIR}/Logger
SOURCES += common/logger.cpp

HEADERS += $${PUBINCLUDEDIR}/State
SOURCES += common/state.cpp

HEADERS += $${PUBINCLUDEDIR}/Progress
SOURCES += common/progress.cpp

HEADERS += $${PUBINCLUDEDIR}/Proxy
SOURCES += common/proxy.cpp

HEADERS += $${PUBINCLUDEDIR}/Watchdog
SOURCES += common/watchdog.cpp

HEADERS += common/config.h
SOURCES += common/config.cpp

HEADERS += common/message.h
SOURCES += common/message.cpp

HEADERS += $${PUBINCLUDEDIR}/Json
SOURCES += common/json/json.cpp

# old GPL Json implementation (small: self-contained)
contains(CONFIG, qtjson) {
    HEADERS += common/json/qtjson.h
    SOURCES += common/json/qtjson.cpp
    SOURCES += common/json/json-qtjson.cpp
}

# new/default LGPL Json implementation (larger: external package)
contains(CONFIG, qjson) {
    SOURCES += common/json/json-qjson.cpp
    LIBS += -lqjson
}

SOURCES += common/utils.cpp

# crypto
!wince {
    SOURCES += common/crypto/crypto.cpp
}
HEADERS += $${PUBINCLUDEDIR}/simplecrypt/simplecrypt.h
SOURCES += common/crypto/simplecrypt.cpp

!nocloud {
    HEADERS += $${PUBINCLUDEDIR}/Vpn/Vpn*
    SOURCES += $${_PRO_FILE_PWD_}/common/vpn/*.cpp
    DEPENDPATH += $${_PRO_FILE_PWD_}/common/vpn
}

#
# core sources
# 

HEADERS += $${PUBINCLUDEDIR}/Manager/NetworkManager
HEADERS += $${PUBINCLUDEDIR}/Manager/ProcessManager
HEADERS += $${PUBINCLUDEDIR}/Manager/Process
SOURCES += $${_PRO_FILE_PWD_}/mgr/*.cpp
DEPENDPATH += $${_PRO_FILE_PWD_}/mgr

HEADERS += $${PUBINCLUDEDIR}/Core
SOURCES += core/core.cpp

#
# setup sources
# 

!nosetup {
    HEADERS += setup/setup.h
    SOURCES += setup/setup.cpp
    modeqcc {
        HEADERS += $${_PRO_FILE_PWD_}/setup/qcc/*.h
        SOURCES += $${_PRO_FILE_PWD_}/setup/qcc/*.cpp
        HEADERS += $${_PRO_FILE_PWD_}/setup/reg/*.h
        SOURCES += $${_PRO_FILE_PWD_}/setup/reg/*.cpp
        DEPENDPATH += $${_PRO_FILE_PWD_}/setup/qcc
        DEPENDPATH += $${_PRO_FILE_PWD_}/setup/reg
    }
    modevpn {
        HEADERS += $${_PRO_FILE_PWD_}/setup/vpn/*.h
        SOURCES += $${_PRO_FILE_PWD_}/setup/vpn/*.cpp
        DEPENDPATH += $${_PRO_FILE_PWD_}/setup/vpn
    }
}


#
# cloud sources
# 

!nocloud {
    HEADERS += $${_PRO_FILE_PWD_}/cloud/*.h
    SOURCES += $${_PRO_FILE_PWD_}/cloud/*.cpp
    DEPENDPATH += $${_PRO_FILE_PWD_}/cloud
}

#
# api sources
# 

!noapi {
    HEADERS += $${PUBINCLUDEDIR}/Server
    HEADERS += $${PUBINCLUDEDIR}/Api/AuthApi
    HEADERS += $${PUBINCLUDEDIR}/Api/CloudApi
    HEADERS += $${PUBINCLUDEDIR}/Api/CommonApi
    HEADERS += $${PUBINCLUDEDIR}/Api/ConfigApi
    HEADERS += $${PUBINCLUDEDIR}/Api/SetupApi
    HEADERS += $${PUBINCLUDEDIR}/Api/StatusApi
    SOURCES += $${_PRO_FILE_PWD_}/api/*.cpp
    DEPENDPATH += $${_PRO_FILE_PWD_}/api
}

#
# http sources
# 

HEADERS += $${PUBINCLUDEDIR}/Http/Http
HEADERS += $${PUBINCLUDEDIR}/Http/HttpHandler
HEADERS += $${PUBINCLUDEDIR}/Http/HttpAbstractHandler
HEADERS += $${PUBINCLUDEDIR}/Http/HttpRequest
HEADERS += $${PUBINCLUDEDIR}/Http/HttpResponse
HEADERS += $${PUBINCLUDEDIR}/Http/HttpHeaders
HEADERS += $${PUBINCLUDEDIR}/Http/HttpServer
HEADERS += $${_PRO_FILE_PWD_}/http/*.h
SOURCES += $${_PRO_FILE_PWD_}/http/*.cpp
DEPENDPATH += $${_PRO_FILE_PWD_}/http

# other libs
about {
    LIBS += $${ABOUT_LIBS}
}

# installation
target.path = $$LIBDIR
INSTALLS += target
