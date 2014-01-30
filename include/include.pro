include(../common.pri)

TEMPLATE = subdirs

HEADERS += encloud/common.h
HEADERS += encloud/core.h
HEADERS += encloud/setup.h
HEADERS += encloud/state.h
HEADERS += encloud/vpn.h

win32 {
    HEADERS += encloud/stdbool.h
}

# header installation
headers.path = $${INCDIR}/encloud
headers.files = $${HEADERS}
INSTALLS += headers
