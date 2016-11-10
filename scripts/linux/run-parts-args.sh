#!/bin/sh

RUN_PARTS="/usr/bin/run-parts"

DIR=$1
shift

ARGS=""
for arg in $@; do
    ARGS="${ARGS} -a ${arg}"
done

${RUN_PARTS} ${ARGS} ${DIR}
