include(../../common.pri)

TEMPLATE = subdirs

scripts.path = $${BINDIR}
scripts.files += openvpn-up.bat
scripts.files += openvpn-down.bat
scripts.files += openvpn-route-up.bat

INSTALLS += scripts
