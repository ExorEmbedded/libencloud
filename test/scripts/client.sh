#!/bin/sh -x

[ $# -lt 1 ] && echo "Usage: $0 <action>" && exit 1

HOST=localhost
URL=api_v1
PORT=4884
ACTION=$1
ARGS=

case ${ACTION} in
    get)
        URL=${URL}/status
        ;;
    setlic)
        URL=${URL}/setup
        ARGS="--post-data 'license=foobar'"
        ;;
esac

wget -O - ${ARGS} ${HOST}:${PORT}/${URL} 2>/dev/null
[ $? -ne 0 ] && echo "error contacting server!"

exit 0
