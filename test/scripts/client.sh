#!/bin/sh

[ $# -lt 1 ] && echo "Usage: $0 <action>" && exit 1

HOST="localhost"
URL="api_v1"
PORT="4884"
ACTION="$1"
LICENSE="94c97e4b-ab8c-4dd6-b06b-ef3e18ed2d83"
LOGINURL="$2"
AUTHTYPE="auth"
USER="$3"
PASS="$4"
ARGS=

case ${ACTION} in
    get)
        URL=${URL}/status
        ;;
    lic)
        URL=${URL}/setup
        ARGS="--post-data license=${LICENSE}"
        ;;
    auth)
        URL=${URL}/auth
        ARGS="--post-data type=${AUTHTYPE}&url=${LOGINURL}&user=${USER}&pass=${PASS}"
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
