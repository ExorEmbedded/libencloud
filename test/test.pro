include(../common.pri)

TEMPLATE = subdirs

!wince {
    !nodll {
        !modevpn {
            SUBDIRS += unit
            SUBDIRS += unit-sec
        }
    }
}

# do nothing
check.commands = true
