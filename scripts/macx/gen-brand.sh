#!/bin/sh
#
# Notes:
#   - escape \${var} as => \\${var}

echo "Generating brand-specific OpenVPN scripts for OSX"

ROUTE_UP_SCRIPT="openvpn-route-up.sh"

# Setup env for paths
if [ "${PACKAGE_XBRAND}" = "true" ]; then
    APPDIR="/Applications/${PACKAGE_NAME}.app"
else
    APPDIR="/Applications/${PACKAGE_ORG}/${PACKAGE_NAME}.app"
fi
BINDIR="${APPDIR}/Contents/MacOS"

cat << EOF > ${ROUTE_UP_SCRIPT}
#!/bin/sh
#
# *GENERATED* by 'libencloud/scripts/macx/$(basename $0)'
#
# OpenVPN 'up' script for OSX:
#  1) Sets Encloud gateway to be used when adding and deleting routes
#    (Switchboard's "openvpn_internal_ip" is unreliable on multicore [CONNECT-377])
#  2) Run fix-routes-osx.sh to fix split routes
#

log()
{
   echo \$@
}

die()
{
    log \$@
    exit 1
}

#
# [1]
#
[ -z \${route_vpn_gateway} ] && die "CRITICAL: 'route_vpn_gateway' variable must be defined!"

#
# Read port from OSX Defaults
#
ENCLOUD_PORT=\$(defaults read "/Library/Preferences/com.${PACKAGE_ORG}.libencloud.plist" port)
log "Read Encloud Port: \${ENCLOUD_PORT}"

#
# Set gateway in Encloud Service
#
"${BINDIR}/qurl" \\
        http://localhost:\${ENCLOUD_PORT}/api/v1/cloud \\
        -d action=setGateway -d ip=\${route_vpn_gateway}
[ \$? -ne 0 ] && die "ERROR: qurl command exited with code: \$?"

#
# [2]
#
"${BINDIR}/fix-routes-osx.sh"

exit 0
EOF

chmod a+x ${ROUTE_UP_SCRIPT}

echo "Done"
exit 0
