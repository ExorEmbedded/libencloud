include(../common.pri)

TEMPLATE = subdirs

unix:SUBDIRS += unit

!nogui:SUBDIRS += sece

# do nothing
check.commands = true
