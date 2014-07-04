include(../common.pri)

TEMPLATE = lib

TARGET = encloud

win32 {
    CONFIG += dll
    # disable number in mingw output (libencloud0.dll vs libencloud.dll)
    TARGET_EXT = .dll

    TARGET = encloud$${DBG_SUFFIX}
} else {
    CONFIG += shared
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

HEADERS += $${PUBINCLUDEDIR}/Error
SOURCES += common/error.cpp

HEADERS += $${PUBINCLUDEDIR}/Info
SOURCES += common/info.cpp

HEADERS += $${PUBINCLUDEDIR}/Json
SOURCES += common/json.cpp

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

# old GPL Json implementation (small: self-contained)
contains(CONFIG, qtjson) {
    HEADERS += common/qtjson.h
    SOURCES += common/qtjson.cpp
    SOURCES += common/json-qtjson.cpp
}

# new/default LGPL Json implementation (larger: external package)
contains(CONFIG, qjson) {
    SOURCES += common/json-qjson.cpp
    LIBS += -lqjson
}

HEADERS += common/utils.h
SOURCES += common/utils.cpp

HEADERS += common/crypto.h
SOURCES += common/crypto.cpp

!nocloud {
    HEADERS += common/vpn/*.h
    SOURCES += common/vpn/*.cpp
}

#
# core sources
# 

HEADERS += $${PUBINCLUDEDIR}/Manager/NetworkManager
HEADERS += $${PUBINCLUDEDIR}/Manager/ProcessManager
HEADERS += $${PUBINCLUDEDIR}/Manager/Process
SOURCES += mgr/*.cpp

HEADERS += $${PUBINCLUDEDIR}/Core
SOURCES += core/core.cpp

#
# setup sources
# 

!nosetup {
    HEADERS += setup/setup.h
    SOURCES += setup/setup.cpp
    modeqic {
        HEADERS += setup/qic/*.h
        SOURCES += setup/qic/*.cpp
    }
    modeece | modesece {
        HEADERS += setup/ece/*.h
        SOURCES += setup/ece/*.cpp
    }
}

#
# cloud sources
# 

!nocloud {
    HEADERS += cloud/*.h
    SOURCES += cloud/*.cpp
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
    HEADERS += api/*.h
    SOURCES += api/*.cpp
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
HEADERS += http/*.h
SOURCES += http/*.cpp

# for SHGetFolderPath()
win32 {
    LIBS += -lshfolder

    PRE_TARGETDEPS  += $$SRCBASEDIR/about/$$DESTDIR/about$${DBG_SUFFIX}.lib

    LIBS += $$PRE_TARGETDEPS
} 

# installation
target.path = $$LIBDIR
INSTALLS += target
