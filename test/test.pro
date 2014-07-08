include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += unit
SUBDIRS += unit-sec

!nogui:SUBDIRS += sece

# do nothing
check.commands = true
