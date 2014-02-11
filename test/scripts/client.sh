#!/bin/sh

[ $# -lt 1 ] && echo "Usage: $0 <action>" && exit 1

HOST=localhost
URL=/test
PORT=8081
ACTION=$1

wget -O - ${HOST}:${PORT}/${URL} 2>/dev/null
[ $? -ne 0 ] && echo "error contacting server!"

exit 0
