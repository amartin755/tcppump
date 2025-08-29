#/usr/bin/bash

device1="pump0"
device2="pump1"

if [ -e "/sys/class/net/${device1}" ]
then
    sudo ip link del ${device1}
fi
