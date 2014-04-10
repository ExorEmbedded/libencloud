include(../common.pri)

TEMPLATE = subdirs

HEADERS += encloud/Auth
HEADERS += encloud/Common
HEADERS += encloud/Core
HEADERS += encloud/Error
HEADERS += encloud/Info
HEADERS += encloud/Logger
HEADERS += encloud/Progress
HEADERS += encloud/Proxy
HEADERS += encloud/Server
HEADERS += encloud/State
HEADERS += encloud/Watchdog

HEADERS += encloud/Http/Http
HEADERS += encloud/Http/HttpAbstractHandler
HEADERS += encloud/Http/HttpHandler
HEADERS += encloud/Http/HttpHeaders
HEADERS += encloud/Http/HttpRequest
HEADERS += encloud/Http/HttpResponse
HEADERS += encloud/Http/HttpServer

HEADERS += encloud/Api/AuthApi
HEADERS += encloud/Api/CloudApi
HEADERS += encloud/Api/CommonApi
HEADERS += encloud/Api/SetupApi
HEADERS += encloud/Api/StatusApi

# header installation
unix {
    headers.path = $${INCDIR}/encloud
    headers.files = $${HEADERS}
    INSTALLS += headers

} # win32 uses relative paths - headers not installed
