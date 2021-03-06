#!/bin/bash
#
# Script to sanitize and apply OpenVPN routes:
# * Avoids problems with split networks using tap device on OSX
#   (possibly related: https://discussions.apple.com/message/8546386)
#
# Implementation Notes:
# * bad split net routes are fixed via -iface parameter
# * --route-noexec is longer used (to unblock features such as
#   --redirect-gateway), so bad split net routes are deleted via delroute()
#   before being fixed with addroute()

this_path=$(dirname "$0")

# disable strict consistency checks to speed up processing
NUTS_OPTIMIZE=1
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
delroute ()
{
    [ "$1" = "" ] && return 0
    [ "$2" = "" ] && return 0

    "${route_path}" delete -net $1 $2
}

# $1: cidr network
# $2: gateway
# $3: is_supernet
addroute ()
{
    [ "$1" = "" ] && return 0
    [ "$2" = "" ] && return 0

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

nuts_info "Sorted CIDRs: $cidrs_sorted"

# second run appends gateway
cidrs_gw=""
for cidr in ${cidrs_sorted}; do
    cidrs_gw="${cidrs_gw} ${cidr}_${route_vpn_gateway}"
done

nuts_info "CIDRs_GW: $cidrs_gw"

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

        if [ "$issuper_prev" = "1" ]; then
            delroute $cidr_prev $gw_prev
            addroute $cidr_prev $gw_prev $issuper_prev
        fi
    fi

    cidr_prev=$cidr
    gw_prev=$gw
    issuper_prev=$issuper
done

if [ "$issuper_prev" = "1" ]; then
    delroute "$cidr_prev" "$gw_prev"
    addroute "$cidr_prev" "$gw_prev" $issuper_prev
fi

nuts_info "$0 OK"

exit 0
