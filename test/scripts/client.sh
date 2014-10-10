#!/bin/sh  -x

[ $# -lt 2 ] && echo "Usage: $0 URL PATH [PARAMS]" && exit 1

URL="$1"
PATH="api/v1/$2"
WGET=/usr/bin/wget

shift
shift

PARAMS=$@

[ "${PARAMS}" != "" ] && ARGS="--post-data ${PARAMS}"

${WGET} -O - ${ARGS} ${URL}/${PATH} 2>/dev/null
[ $? -ne 0 ] && echo "error contacting server!"

# in case content has no newline
echo

exit 0
