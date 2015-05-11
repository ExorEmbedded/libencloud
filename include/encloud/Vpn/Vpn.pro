include(../../../common.pri)

TEMPLATE = subdirs

HEADERS += VpnClient
HEADERS += VpnConfig
HEADERS += VpnManager

# header installation
unix {
    headers.path = $${INCDIR}/encloud/Vpn
    headers.files = $${HEADERS}
    INSTALLS += headers

} # win32 uses relative paths - headers not installed
