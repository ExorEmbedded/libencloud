include(../common.pri)

TEMPLATE = subdirs

HEADERS += encloud/Auth
HEADERS += encloud/Common
HEADERS += encloud/Core

HEADERS += encloud/Http
HEADERS += encloud/HttpAbstractHandler
HEADERS += encloud/HttpHandler
HEADERS += encloud/HttpHeaders
HEADERS += encloud/HttpRequest
HEADERS += encloud/HttpResponse
HEADERS += encloud/HttpServer

HEADERS += encloud/Info
HEADERS += encloud/Logger
HEADERS += encloud/Progress
HEADERS += encloud/State

HEADERS += encloud/Api/Auth
HEADERS += encloud/Api/Common
HEADERS += encloud/Api/Setup
HEADERS += encloud/Api/Status

# header installation
unix {
    headers.path = $${INCDIR}/encloud
    headers.files = $${HEADERS}
    INSTALLS += headers

} # win32 uses relative paths - headers not installed
