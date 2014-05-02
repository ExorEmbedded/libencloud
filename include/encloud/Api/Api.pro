include(../../../common.pri)

TEMPLATE = subdirs

HEADERS += AuthApi
HEADERS += CloudApi
HEADERS += CommonApi
HEADERS += SetupApi
HEADERS += StatusApi

# header installation
unix {
    headers.path = $${INCDIR}/encloud/Api
    headers.files = $${HEADERS}
    INSTALLS += headers

} # win32 uses relative paths - headers not installed
