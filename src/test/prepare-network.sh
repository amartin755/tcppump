#/usr/bin/bash

device1="veth0"
device2="veth1"

if [ ! -e "/sys/class/net/${device1}" ] && [ ! -e "/sys/class/net/${device2}" ]
then
    # create virtual ethernet devices
    sudo ip link add veth0 type veth peer name veth1
    sudo ip link set veth0 up
    sudo ip link set veth1 up

    # disable ipv6
    sudo sysctl net.ipv6.conf.veth0.disable_ipv6=1
    sudo sysctl net.ipv6.conf.veth1.disable_ipv6=1
fi
