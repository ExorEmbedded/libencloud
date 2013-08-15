#!/bin/sh -x
#
# Simple script to run test under gdb or strace

[ $# -eq 1 ] || exit 1

if [ "$1" = "g" ]; then
    WRAPPER=gdb
elif [ "$1" = "s" ]; then
    WRAPPER=strace
else
    exit 2
fi

export LD_LIBRARY_PATH=../src:${HOME}/inst/lib 

${WRAPPER} ./test
