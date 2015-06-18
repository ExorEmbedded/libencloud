#!/bin/bash -e

echo "Running custom route-up script"

n_nets=$(env | grep ^route_network_ | wc -l | tr -d ' ')

for ((i = 1; i <= ${n_nets}; i++)); do
    network=$(eval echo \${route_network_${i}})
    gateway=$(eval echo \${route_gateway_${i}})
    netmask=$(eval echo \${route_netmask_${i}})

    ARGS="add";
    ARGS="${ARGS} -net ${network}"
    ARGS="${ARGS} ${gateway}"
    ARGS="${ARGS} ${netmask}"

    # Avoids problems with split networks using tap device on OSX
    # (possibly related: https://discussions.apple.com/message/8546386)
    [ "${netmask}" = "255.255.255.128" ] && ARGS="${ARGS} -iface"
    [ "${netmask}" = "255.255.128.0" ] && ARGS="${ARGS} -iface"

    /sbin/route ${ARGS}
done

exit 0
