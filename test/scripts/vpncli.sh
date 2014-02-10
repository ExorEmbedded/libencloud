#!/bin/sh -x

CONF_DIR="../etc"
SUDO_CMD="sudo"
OVPN_CMD="openvpn"
OVPN_CONF="${CONF_DIR}/openvpn-cli.conf"
OVPN_MGMT="true"
OVPN_MGMT_PORT="1195"

OVPN_ARGS=""

if [ "${OVPN_MGMT}" = "true" ]; then
    OVPN_ARGS="${OVPN_ARGS} --management localhost ${OVPN_MGMT_PORT}"
    OVPN_ARGS="${OVPN_ARGS} --management-forget-disconnect"
    OVPN_ARGS="${OVPN_ARGS} --management-query-passwords"
fi

OVPN_ARGS="${OVPN_ARGS}  --config ${OVPN_CONF}"

${SUDO_CMD} ${OVPN_CMD} ${OVPN_ARGS}
