#/usr/bin/bash

device1="veth0"
device2="veth1"

if [ ! -e "/sys/class/net/${device1}" ] && [ ! -e "/sys/class/net/${device2}" ]
then
    # create virtual ethernet devices
    sudo ip link add ${device1} type veth peer name ${device2}

    # set mac address
    sudo ip link set dev ${device1} address 12:01:01:01:01:01
    sudo ip link set dev ${device2} address 22:02:02:02:02:02
    # set ipv4 address
    sudo ip addr add 10.1.1.1/24 dev ${device1}
    sudo ip addr add 10.2.2.2/24 dev ${device2}
    # enable interfaces
    sudo ip link set ${device1} up
    sudo ip link set ${device2} up

    # disable ipv6
    sudo sysctl net.ipv6.conf.${device1}.disable_ipv6=1
    sudo sysctl net.ipv6.conf.${device2}.disable_ipv6=1
fi
