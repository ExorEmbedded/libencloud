include(../../common.pri)

TEMPLATE = subdirs

HEADERS += Auth
HEADERS += Client
HEADERS += Common
HEADERS += Core
!wince {
    HEADERS += Crypto
}
HEADERS += Error
HEADERS += Info
HEADERS += Json
HEADERS += Logger
HEADERS += Progress
HEADERS += Proxy
HEADERS += Server
HEADERS += State
HEADERS += Utils
HEADERS += Watchdog

SUBDIRS += Api
SUBDIRS += Http
SUBDIRS += Manager
SUBDIRS += Vpn

# header installation
unix {
    headers.path = $${INCDIR}/encloud
    headers.files = $${HEADERS}
    INSTALLS += headers

} # win32 uses relative paths - headers not installed
