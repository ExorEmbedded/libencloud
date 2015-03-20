include(../../common.pri)

TEMPLATE = subdirs

scripts.path = $${BINDIR}
scripts.files = route-up.sh

INSTALLS += scripts
