#!/bin/sh

[ $# -lt 1 ] && echo "Usage: $0 <action>" && exit 1

HOST=localhost
PORT=8081
ACTION=$1

echo '{ "action" : "'${ACTION}'" }' | nc -q 3 ${HOST} ${PORT}
