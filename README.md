[![master: build-and-test](https://github.com/amartin755/tcppump/actions/workflows/build-and-test.yml/badge.svg?branch=master)](https://github.com/amartin755/tcppump/actions/workflows/build-and-test.yml)
[![devel: build-and-test](https://github.com/amartin755/tcppump/actions/workflows/build-and-test.yml/badge.svg?branch=devel)](https://github.com/amartin755/tcppump/actions/workflows/build-and-test.yml)

# tcppump
Simple Ethernet network packet generator

tcpdump is a simple command-line tool for generating Ethernet packets, with focus on low level protocols.

Features
* Completely operating system independent packet generation with full control of all protocol details of each packet. Thus it is also possible to generate deliberately malformed packets.
* Definiton of packets via a token based syntax
* Transmission of packets either directly via command line parameters or script based
* Time triggered transmission of packets (realtime mode). Note: Accuracy depends on host environment.
* Loops
* User defined delays between packets
* Realtime replaying of PCAP files
* Source MAC and IPv4 address can be freely defined (using the addresses of the network adapter is also possible)
* Automatic ARP resolution of unknown hosts
* Random source and destination MAC addresses
* Output of generated network traffic to PCAP files
* Responder mode allows mirroring of received packets or receive triggered transmission of custom packets

Supported protocols and packet formats
* Raw packets as bytestream
* Ethernet II packets
* IEEE802.3 packets, with IEEE802.2 header (aka LLC) including SNAP
* VLAN tagging; including multiple tagging
* ARP
* Raw IPv4
* UDP
* TCP
* VRRP
* STP/RSTP
* IGMP (v1/v2 only, v3 is on the way)
* ICMP
* VXLAN
* GRE

**Note: This tool is in alpha state.**

## Build instructions
tcppump has been successfully compiled on ubuntu linux (x64) with gcc and on windows 10/11 with mingw32, mingw64(MSYS2) and MSVC.
Theoretically it should also be compilable with other C++11 compilers, but this was never tested.

### Dependencies
* cmake >= V3.13
* C++11 compiler

Windows
* WinPcap
* WinPcap Developer' Pack (see http://www.winpcap.org/devel.htm and http://www.winpcap.org/install/bin/WpdPack_4_1_2.zip)

Linux
* libpcap
* libpcap development files (libpcap-devel)

### How to build
#### Setup

    cd tcpdump
    cmake -B build

If you use 'msys' and 'make' add the following cmake parameter:

    -G "MSYS Makefiles"

For debugging add:

    -DCMAKE_BUILD_TYPE=Debug

#### Build

    cmake --build build


## How to use
### Command-line interface
    Usage: tcppump -i IFC [OPTIONS] packets/infiles

    Options:
    -h --help
            Display this text
    --version
            Show detailed version infos
    -i IFC --interface=IFC
            Name of the network interface via which the packets are sent.
    --myip4=IPV4
            Use IPV4 as source IPv4 address instead of the network adapters ip address
    --mymac=MAC
            Use MAC as source MAC address instead of the network adapters MAC address
    --mtu=MTU
            Use MTU instead of the network adapters mtu
    --rand-smac
            Use random source MAC address. Overwrites --mymac as well as explicitly defined addresses in packets.
    --rand-dmac
            Use random destination MAC address. Overwrites all explicitly defined addresses in packets.
    --overwrite-dmac=MAC
            Overwrite destination MAC address of all packets to MAC
    -v --verbose
            When parsing and printing, produce verbose output. This option can be supplied multiple times
            (max. 4 times, i.e. -vvvv) for even more debug output.
    -s --script
            Packets are defined in script files, that contain token based packets.
    --pcap[=SCALE]
            pcap file of captured packets (e.g via wireshark or tcpdump) will be replayed.
            The transmission time can be scaled via the optional parameter SCALE.
            Default value of SCALE is 1.0, which means the file is played in realtime.
            For example a value of 2.0 means it is played half as fast, 0.5 means twice as fast.
            If SCALE is 0 the file will be played as fast as possible.
    -l N --loop=N
            Send all files/packets N times. Default: N = 1. If N = 0, packets will be sent infinitely
            until ctrl+c is pressed.
    -d TIME --delay=TIME
            Packet transmission is delayed TIME.Resolution depends on -t parameter. Default is no delay.
    -t RESOLUTION --resolution=RESOLUTION
            Resolution of transmission time. This affects -d parameter as well as all timestamps in script files.
            Possible values are 'u'= microseconds, 'm'= milliseconds(default), 'c'= centiseconds and 's'= seconds
    -o OUTFILE --write-to-file=OUTFILE
            Write generated packets to pcap file OUTFILE instead of sending them to the network.
    -a --arp
            Resolve destination MAC address for IPv4 packets.
            If dmac parameter of IPv4 based packets is omitted, the destination MAC will be automatically
            determined via ARP.
    --listener=MODE
            Enable responder mode (EXPERIMENTAL). Possible values for MODE are:
            mirror  Each received packet will be mirrored back to the sender.
            trigger Each received packet will trigger sending of specified packets.
    --bpf-filter=FILTER
            Receive bpf filter for responder mode.
    --predictable-random
            Don't use random numbers, use simple sequence instead.

### Examples

Raw packet

    tcppump -i eth0 "raw(payload=1122334455667788990011223344)"

VLAN tagged packet (default tag values)

    tcppump -i eth0 "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=1, ethertype=0x8123, payload=1234567890abcdef)"

VLAN tagged packet

    tcppump -i eth0 "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=42, prio=3, ethertype=0x8123, payload=1234567890abcdef)"

IEEE802.3 packet with LLC header

    tcppump -i eth0 "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, dsap=0x12, ssap=0x34, control=0x11, payload=1122)"

IEEE802.3 packet with LLC header and VLAN tag

    tcppump -i eth0 "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vlan=1, vid=42, prio=3, dsap=0x12, ssap=0x34, control=0x11, payload = 1122)"

SNAP packet (IEEE802.3 packet with LLC header and SNAP extension)

    tcppump -i eth0 "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, oui=0x808182, protocol=0x34, payload=1234567890abcdef)"

VLAN double tagged packet

    tcppump -i eth0 "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=100, vtype=2, vid=42, prio=3, ethertype=0x8123, payload=1234567890abcdef)"

Simple ARP (who has IP 11.22.33.44?)

    tcppump -i eth0 "arp(dip=11.22.33.44)"

Fully defined ARP packet

    tcppump -i eth0 "arp(op=1, smac=10:22:33:44:55:66, sip=192.168.0.166, dmac=01:02:03:04:05:06, dip=1.2.3.4)"

Simple ARP combined with VLAN tag

    tcppump -i eth0 "arp(vid=123, dip=11.22.33.44)"

Raw simple IPv4 packet

    tcppump -i eth0 "ipv4(dmac=11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678)"

Raw full defined IPv4 packet

    tcppump -i eth0 "ipv4(smac=80:12:34:45:67:89, dmac = 11:22:33:44:55:66, sip=192.168.0.1, dip=172.16.1.2, ttl=200, dscp=16, ecn=1, df=1, protocol=254, payload=12345678)"

UDP packet

    tcppump -i eth0 "udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=12345678)"
