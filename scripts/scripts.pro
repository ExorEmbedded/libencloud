include(../common.pri)

TEMPLATE = subdirs

win32 {
    SUBDIRS += win32
}

macx {
    SUBDIRS += macx
}

linux-g++ {
    SUBDIRS += linux
}
