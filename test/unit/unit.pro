include(../../common.pri)

TEMPLATE = app

# external prog - don't exports dlls
DEFINES -= _LIBENCLOUD_LIB_

TARGET = unit

CONFIG += gui
CONFIG += console
CONFIG += qtestlib
CONFIG -= app_bundle

SOURCES += *.cpp
HEADERS += *.h

INCLUDEPATH += $$SRCBASEDIR/src/
DEPENDPATH += $$SRCBASEDIR/src/

INCLUDEPATH += /Users/stewy/local/src/qtyaml/src
LIBS += -L/Users/stewy/local/src/qtyaml/bin/release -lqtyaml

# libencloud
LIBS += -L$$SRCBASEDIR/src/$$DESTDIR
LIBS += -lencloud$$DBG_SUFFIX
about {
    LIBS += $${ABOUT_LIBS}
}

# json - external linkage only for QJson
contains(CONFIG, qjson) {
    LIBS += -lqjson
}

# command to run upon 'make check'
# LIBENCLOUD_WRAP environment variable can be set to "gdb", "valgrind", etc
macx {
    check.commands = DYLD_LIBRARY_PATH=:$$SRCBASEDIR/src:/Users/stewy/local/src/qtyaml/bin/release:$$SRCBASEDIR/about:$$LIBDIR $$(LIBENCLOUD_WRAP) ./$$TARGET
} else {
    check.commands = LD_LIBRARY_PATH=:$$SRCBASEDIR/src:$$SRCBASEDIR/about:$$LIBDIR $$(LIBENCLOUD_WRAP) ./$$TARGET
}

target.path = $${BINDIR}
INSTALLS += target

unix:!macx {
    post.path = $${BINDIR}
    post.extra = $${QMAKE_MOVE} "\"${INSTALL_ROOT}/$${BINDIR}/$${TARGET}\"" "\"${INSTALL_ROOT}/$${BINDIR}/libencloud-test\""
    INSTALLS += post
}

nodll {
    DEFINES += LIBENCLOUD_DLLSPEC=
    DEFINES += LIBENCLOUDABOUT_DLLSPEC=
    # for SHGetFolderPath()
    win32 {
        !wince {
            LIBS += -lshfolder
        }
    }

    PRE_TARGETDEPS  += $$OUT_PWD/../../about/$$DESTDIR/about$${DBG_SUFFIX}.lib

    LIBS += $$PRE_TARGETDEPS
}
