include(../../common.pri)

TEMPLATE = subdirs

scripts.path = $${BINDIR}
scripts.files += run-parts-args.sh

INSTALLS += scripts
