#!/bin/sh

[ $# -lt 2 ] && echo "Usage: $0 HOST ACTION [URL] [USER] [PASS]" && exit 1

HOST="$1"
URL="api_v1"
PORT="4884"
ACTION="$2"
LICENSE="94c97e4b-ab8c-4dd6-b06b-ef3e18ed2d83"
LOGINURL="$3"
USER="$4"
PASS="$5"
ARGS=

case ${ACTION} in
    get)
        URL=${URL}/status
        ;;
    lic)
        URL=${URL}/setup
        ARGS="--post-data license=${LICENSE}"
        ;;
    sb_auth)
        URL=${URL}/auth
        ARGS="--post-data id=sb&url=${LOGINURL}&user=${USER}&pass=${PASS}"
        ;;
    proxy_auth)
        URL=${URL}/auth
        ARGS="--post-data id=proxy&type=http&url=${LOGINURL}&user=${USER}&pass=${PASS}"
        ;;
    *)
        echo "Invalid command: '${ACTION}'!"
        exit 1
esac

wget -O - ${ARGS} ${HOST}:${PORT}/${URL} 2>/dev/null
[ $? -ne 0 ] && echo "error contacting server!"

# in case content has no newline
echo

exit 0
