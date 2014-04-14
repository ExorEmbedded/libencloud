#!/bin/sh -x

CONF_DIR="../etc"
SUDO_CMD="sudo"
OVPN_CMD="openvpn"
OVPN_CONF="${CONF_DIR}/openvpn-srv.conf"

OVPN_ARGS=""
OVPN_ARGS="${OVPN_ARGS}  --config ${OVPN_CONF}"

${SUDO_CMD} ${OVPN_CMD} ${OVPN_ARGS}
