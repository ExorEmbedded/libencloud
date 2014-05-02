include(../../../common.pri)

TEMPLATE = subdirs

HEADERS += Http
HEADERS += HttpAbstractHandler
HEADERS += HttpHandler
HEADERS += HttpHeaders
HEADERS += HttpRequest
HEADERS += HttpResponse
HEADERS += HttpServer

# header installation
unix {
    headers.path = $${INCDIR}/encloud/Http
    headers.files = $${HEADERS}
    INSTALLS += headers

} # win32 uses relative paths - headers not installed
