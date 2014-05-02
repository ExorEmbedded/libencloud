include(../../../common.pri)

TEMPLATE = subdirs

HEADERS += NetworkManager
HEADERS += Process
HEADERS += ProcessManager

# header installation
unix {
    headers.path = $${INCDIR}/encloud/Manager
    headers.files = $${HEADERS}
    INSTALLS += headers

} # win32 uses relative paths - headers not installed

