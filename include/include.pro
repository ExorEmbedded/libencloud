include(../common.pri)

TEMPLATE = subdirs

HEADERS += encloud.h
win32 {
    HEADERS += stdbool.h
}

# header installation
headers.path = $${INCDIR}
headers.files = $${HEADERS}
INSTALLS += headers
