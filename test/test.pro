include(../common.pri)

TEMPLATE = subdirs

SUBDIRS += unit

!nogui:SUBDIRS += sece

# do nothing
check.commands = true
