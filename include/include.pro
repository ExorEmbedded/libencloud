include(../common.pri)

TEMPLATE = subdirs

HEADERS += ece.h

# header installation
headers.path = $${INCDIR}
headers.files = $${HEADERS}
INSTALLS += headers
