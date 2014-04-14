#!/bin/sh  -x

[ $# -lt 2 ] && echo "Usage: $0 HOST PATH [PARAMS]" && exit 1

HOST="$1"
PATH="api_v1/$2"
PORT="4804"
WGET=/usr/bin/wget

shift
shift

PARAMS=$@

[ "${PARAMS}" != "" ] && ARGS="--post-data ${PARAMS}"

${WGET} -O - ${ARGS} ${HOST}:${PORT}/${PATH} 2>/dev/null
[ $? -ne 0 ] && echo "error contacting server!"

# in case content has no newline
echo

exit 0
