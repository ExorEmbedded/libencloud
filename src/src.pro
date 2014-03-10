include(../common.pri)

TEMPLATE = lib

TARGET = encloud

win32 {
    CONFIG += dll
    # disable number in mingw output (libencloud0.dll vs libencloud.dll)
    TARGET_EXT = .dll
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

HEADERS += $$SRCBASEDIR/include/encloud/Auth
SOURCES += common/auth.cpp

HEADERS += $$SRCBASEDIR/include/encloud/Client
SOURCES += common/client.cpp

HEADERS += $$SRCBASEDIR/include/encloud/Info
SOURCES += common/info.cpp

HEADERS += $$SRCBASEDIR/include/encloud/Json
SOURCES += common/json.cpp

HEADERS += $$SRCBASEDIR/include/encloud/Logger
SOURCES += common/logger.cpp

HEADERS += $$SRCBASEDIR/include/encloud/State
SOURCES += common/state.cpp

HEADERS += $$SRCBASEDIR/include/encloud/Progress
SOURCES += common/progress.cpp

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

HEADERS += $$SRCBASEDIR/include/encloud/Core
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
    HEADERS += $$SRCBASEDIR/include/encloud/Server
    HEADERS += $$SRCBASEDIR/include/encloud/Api/AuthApi
    HEADERS += $$SRCBASEDIR/include/encloud/Api/CloudApi
    HEADERS += $$SRCBASEDIR/include/encloud/Api/CommonApi
    HEADERS += $$SRCBASEDIR/include/encloud/Api/SetupApi
    HEADERS += $$SRCBASEDIR/include/encloud/Api/StatusApi
    HEADERS += api/*.h
    SOURCES += api/*.cpp
}

#
# http sources
# 

HEADERS += $$SRCBASEDIR/include/encloud/Http
HEADERS += $$SRCBASEDIR/include/encloud/HttpHandler
HEADERS += $$SRCBASEDIR/include/encloud/HttpAbstractHandler
HEADERS += $$SRCBASEDIR/include/encloud/HttpRequest
HEADERS += $$SRCBASEDIR/include/encloud/HttpResponse
HEADERS += $$SRCBASEDIR/include/encloud/HttpHeaders
HEADERS += $$SRCBASEDIR/include/encloud/HttpServer
HEADERS += http/*.h
SOURCES += http/*.cpp

# for SHGetFolderPath()
win32 {
    LIBS += -lshfolder
}

# installation
target.path = $$LIBDIR
INSTALLS += target
