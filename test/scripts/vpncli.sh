#!/bin/sh
#
# arg   var    description              values                      default
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# $1    PATH   Where to run client      destination directory       <current dir>
# $2    MODE   Mode                     'PLAIN', 'ECE'              'PLAIN'
# $3    ARGS   Extra OpenVPN arguments                              <null>

#
# defaults
#
MODE="PLAIN"
SUDO_CMD="sudo"
OVPN_CMD="openvpn"
OVPN_CONF="openvpn.conf"
OVPN_MGMT="true"
OVPN_MGMT_PORT="1195"
OVPN_ARGS=""

[ $1 ] && RUN_DIR=$1
[ $2 ] && MODE=$2
if [ $3 ]; then
    shift
    shift
    OPENVPN_EXTRA_ARGS=$@
fi

#
# funcs
#

msg()
{
    echo "# $@"
}

die()
{
    msg "$@"
    exit 1
}

wrap()
{
    msg "Executing $@"
    $@
    rc=$?
    [ ${rc} -ne 0 ] && die "Command '$@' returned ${rc} - exiting"
}

cleanup()
{
    [ ${OLDPWD} ] && cd - >/dev/null
}

trap cleanup 0 1 2 3 4 6 7 8 9 11 13 15

#
# main
#

if [ ${RUN_DIR} ]; then
    [ ! -d "${RUN_DIR}" ] && die "Not a valid directory: ${RUN_DIR}!"
    cd "${RUN_DIR}"
fi

if [ "${OVPN_MGMT}" = "true" ]; then
    OVPN_ARGS="${OVPN_ARGS} --management localhost ${OVPN_MGMT_PORT}"
    OVPN_ARGS="${OVPN_ARGS} --management-forget-disconnect"
    OVPN_ARGS="${OVPN_ARGS} --management-query-passwords"
fi

# if MODE is PLAIN we just use config file

if [ "${MODE}" = "ECE" ]; then
    OVPN_ARGS="${OVPN_ARGS} --ca op_ca.pem"
    OVPN_ARGS="${OVPN_ARGS} --key op_key.pem"
    OVPN_ARGS="${OVPN_ARGS} --cert op_cert.pem"
fi

OVPN_ARGS="--config ${OVPN_CONF} ${OVPN_ARGS} ${OPENVPN_EXTRA_ARGS}"

wrap ${SUDO_CMD} ${OVPN_CMD} ${OVPN_ARGS}
