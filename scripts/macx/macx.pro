include(../../common.pri)

TEMPLATE = subdirs

scripts.path = $${BINDIR}
scripts.files += nuts
scripts.files += openvpn-up.sh
scripts.files += openvpn-down.sh
scripts.files += openvpn-route-up.sh

INSTALLS += scripts
