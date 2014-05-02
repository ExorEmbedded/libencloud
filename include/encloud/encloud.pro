include(../../common.pri)

TEMPLATE = subdirs

HEADERS += Auth
HEADERS += Client
HEADERS += Common
HEADERS += Core
HEADERS += Error
HEADERS += Info
HEADERS += Json
HEADERS += Logger
HEADERS += Progress
HEADERS += Proxy
HEADERS += Server
HEADERS += State
HEADERS += Watchdog

SUBDIRS += Api
SUBDIRS += Http
SUBDIRS += Manager

# header installation
unix {
    headers.path = $${INCDIR}/encloud
    headers.files = $${HEADERS}
    INSTALLS += headers

} # win32 uses relative paths - headers not installed
