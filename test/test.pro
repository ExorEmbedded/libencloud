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

!nogui:SUBDIRS += sece

# do nothing
check.commands = true
