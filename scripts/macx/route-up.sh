#!/bin/bash
#
# Avoids problems with split networks using tap device on OSX

echo "Running custom route-up script"

n_nets=$(env | grep ^route_network_ | wc -l | tr -d ' ')

for ((i = 1; i <= ${n_nets}; i++)); do
    network=$(eval echo \${route_network_${i}})
    gateway=$(eval echo \${route_gateway_${i}})
    netmask=$(eval echo \${route_netmask_${i}})
    /sbin/route add -net ${network} -iface ${dev} ${gateway} ${netmask}
done
