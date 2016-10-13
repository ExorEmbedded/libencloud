#!/bin/sh
#
# *GENERATED* by 'libencloud/scripts/macx/gen-brand.sh'
#
# OpenVPN 'up' script for OSX:
#  1) Sets Encloud gateway to be used when adding and deleting routes
#    (Switchboard's "openvpn_internal_ip" is unreliable on multicore [CONNECT-377])
#  2) Run fix-routes-osx.sh to fix split routes
#

log()
{
   echo $@
}

die()
{
    log $@
    exit 1
}

#
# Check preconditions
#
[ -z ${route_vpn_gateway} ] && die "CRITICAL: 'route_vpn_gateway' variable must be defined!"

#
# Read port from OSX Defaults
#
ENCLOUD_PORT=$(defaults read "/Library/Preferences/com.Endian.libencloud.plist" port)
log "Read Encloud Port: ${ENCLOUD_PORT}"

#
# Set gateway in Encloud Service
#
"/Applications/Endian/Endian Connect App.app/Contents/MacOS/qurl" \
        http://localhost:${ENCLOUD_PORT}/api/v1/cloud \
        -d action=setGateway -d ip=${route_vpn_gateway}
[ $? -ne 0 ] && die "CRITICAL: qurl command exited with code: $?"

#
# Run Tunnelblick's up script
#
"/Applications/Endian/Endian Connect App.app/Contents/MacOS/fix-routes-osx.sh"

exit 0
