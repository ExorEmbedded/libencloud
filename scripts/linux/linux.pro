include(../../common.pri)

TEMPLATE = subdirs

QMAKE_STRIP =

scripts.path = $${BINDIR}
scripts.files += run-parts-args.sh

INSTALLS += scripts
