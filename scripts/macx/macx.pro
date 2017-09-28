include(../../common.pri)

TEMPLATE = subdirs

QMAKE_STRIP =

scripts.path = $${BINDIR}
scripts.files += nuts
scripts.files += fix-routes-osx.sh

scripts.files += openvpn-up.sh
scripts.files += openvpn-down.sh
scripts.files += openvpn-route-up.sh

INSTALLS += scripts
