include(../../common.pri)

TEMPLATE = subdirs

QMAKE_STRIP =

scripts.path = $${BINDIR}
scripts.files += $$PWD/run-parts-args.sh

INSTALLS += scripts
