#!/bin/bash
#
# Script to sanitize and apply OpenVPN routes
#
# - Avoids problems with split networks using tap device on OSX
# (possibly related: https://discussions.apple.com/message/8546386)

this_path="$(dirname $0)"
. "${this_path}/nuts"

route_path="/sbin/route"

nuts_info "$0 START"

#
# config
#

#RUNTESTS=1
[ "$RUNTESTS" = "1" ] && nuts_runtests && exit 0

# $1: cidr network
# $2: gateway
# $3: supernet
addroute ()
{
    local args="add -net $1 $2"

    [ "$3" = "1" ] && args="$args -iface"
    
    "${route_path}" ${args}
}

#
# main
#

n_nets=$(env | grep ^route_network_ | wc -l | tr -d ' ')
cidrs=""

# first run converts nets to CIDR format
for ((i = 1; i <= ${n_nets}; i++)); do

    network=$(eval echo \${route_network_${i}})
    netmask=$(eval echo \${route_netmask_${i}})
    cidr=$(nuts_mask2cidr "$netmask")

    cidrs="${cidrs} ${network}/${cidr}"
done

nuts_info "CIDRs: ${cidrs}"

# sort networks
cidrs_sorted=$(nuts_sort ${cidrs})
cidrs_gw=""

# second run sorts original data based on cidrs_sorted
for cs in ${cidrs_sorted}; do

    for ((i = 1; i <= ${n_nets}; i++)); do

        network=$(eval echo \${route_network_${i}})
        netmask=$(eval echo \${route_netmask_${i}})
        gateway=$(eval echo \${route_gateway_${i}})
        cidr=$(nuts_mask2cidr "$netmask")

        if [ "$cs" = "${network}/${cidr}" ]; then
            cidrs_gw="${cidrs_gw} ${cs}_${gateway}"
        fi

    done
    
done

nuts_info "Sorted CIDRs_GW: $cidrs_gw"

cidr_prev=""
gw_prev=""
issuper_prev="0"

# third run checks for supernets and runs route commands
for cg in ${cidrs_gw}; do

    cidr=$(echo $cg | cut -d '_' -f 1)
    gw=$(echo $cg | cut -d '_' -f 2)

    if [ "$cidr_prev" != "" ]; then
        issuper="0"
        super=$(nuts_super $cidr_prev $cidr)
        if [ $? -eq 0 ]; then
            nuts_info "detected supernet: $super"
            issuper="1"
            issuper_prev="1"
        fi
        addroute $cidr_prev $gw_prev $issuper_prev
    fi

    cidr_prev=$cidr
    gw_prev=$gw
    issuper_prev=$issuper
done

addroute $cidr_prev $gw_prev $issuper_prev

nuts_info "$0 OK"

exit 0
