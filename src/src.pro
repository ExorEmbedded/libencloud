include(../common.pri)

TEMPLATE = lib

CONFIG += shared

TARGET = encloud

win32 {
    CONFIG += dll
    # disable number in mingw output (libencloud0.dll vs libencloud.dll)
    TARGET_EXT = .dll
}

#
# common sources
# 

HEADERS += common/common.h
HEADERS += common/defaults.h
HEADERS += common/helpers.h

HEADERS += $$SRCBASEDIR/include/encloud/Info
SOURCES += common/info.cpp

HEADERS += common/config.h
SOURCES += common/config.cpp

HEADERS += common/json.h
SOURCES += common/json.cpp

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
    mode4ic {
        HEADERS += setup/4icsetup.h
        SOURCES += setup/4icsetup.cpp
    }
    modeece | modesece {
        HEADERS += setup/ecesetup.h
        SOURCES += setup/ecesetup.cpp
    }
}

#
# cloud sources
# 

!nocloud {
    HEADERS += cloud/cloud.h
    SOURCES += cloud/cloud.cpp
}

#
# http sources
# 

HEADERS += $$SRCBASEDIR/include/encloud/Http
SOURCES += http/http.cpp

HEADERS += $$SRCBASEDIR/include/encloud/HttpHandler
HEADERS += $$SRCBASEDIR/include/encloud/HttpAbstractHandler
SOURCES += http/handler.cpp

HEADERS += $$SRCBASEDIR/include/encloud/HttpRequest
SOURCES += http/request.cpp

HEADERS += $$SRCBASEDIR/include/encloud/HttpResponse
SOURCES += http/response.cpp

HEADERS += $$SRCBASEDIR/include/encloud/HttpHeaders
SOURCES += http/headers.cpp

HEADERS += $$SRCBASEDIR/include/encloud/HttpServer
SOURCES += http/server.cpp

target.path = $$LIBDIR
INSTALLS += target
