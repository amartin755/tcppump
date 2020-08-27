![Test-on-ubuntu-gcc-64bit](https://github.com/amartin755/tcppump/workflows/Test-on-ubuntu-gcc-64bit/badge.svg)
![Test-on-windows-MSVC-64bit](https://github.com/amartin755/tcppump/workflows/Test-on-windows-MSVC-64bit/badge.svg)

# tcppump
Simple Ethernet network packet generator

tcpdump is a simple command-line tool for generating Ethernet packets, with focus on low level protocols.

Features
* Definiton of packets either as ASCII hex string or via a token based language
* Sending of single packets
* Script defined packets
* Loops
* Delays between packets
* Interactive-Mode (binding packets to key press events)
* Replaying of PCAP files
* Protocol-dissection of sent packets
* Source MAC and IPv4 address can be freely defined (using the addresses of the network adapter is also possible)

Supported protocols and packet formats
* Raw packets as bytestream
* Ethernet II packets
* IEEE802.3 (aka LLC) packets; including SNAP
* VLAN tagging; including multiple tagging
* ARP
* Raw IPv4 (fragmentation is currently not supported)

**Note: This tool is in alpha state. Use at your own risk. Currently there are no binaries available.**

## Build instructions
tcppump has been successfully compiled on win 7 and win 10 (both x64) with mingw32 and on ubuntu linux (x64) with gcc.
Theoretically it should also be compilable with other C++11 compilers (e.g. Visual Studio), but this was never tested.

### Dependencies
* cmake >= V3.13
* C++11 compiler

Windows
* WinPcap
* WinPcap Developer' Pack (see http://www.winpcap.org/devel.htm and http://www.winpcap.org/install/bin/WpdPack_4_1_2.zip)

Linux
* optional: libpcap-devel (for pcap replay feature)

### How to build
#### Setup

    cd tcpdump
    cmake -S src -B build

If cmake can not detect your mingw installation on windows, this cmake parameter can help:

    -G "MinGW Makefiles"

To enable unit test targets add the following cmake paramaters:

    -DWITH_UNITTESTS=1

For debugging add:

    -DCMAKE_BUILD_TYPE=Debug

#### Build

    cmake --build build


## How to use
### Command-line interface
    tcppump -i IFC [OPTIONS] packets/infiles

    Options:
    -h --help
        Display this text
    -i IFC --interface=IFC
        Name of the network interface via which the packets are sent.
    --myip4=IPV4
        Use IPV4 as source IPv4 address instead of the network adapters ip address
    --mymac=MAC
        Use MAC as source MAC address instead of the network adapters MAC address
    -v --verbose
        When parsing and printing, produce verbose output. This option can be supplied multiple times
        (max. 4 times, i.e. -vvvv) for even more debug output.
    --input=TYPE
        Input format of the packets to be sent. Possible values for TYPE (default is "token") are:
        raw     Packets are defined as hex-ascii string, and will not be interpreted.
                example: 0123456789ABCDEF001122334455667788
        token   Token based definition of packets. tcppump will compile it to Ethernet packets.
                example: eth: .dest=44:22:33:44:55:66 .payload=1234567890abcdef
                For complete description of the syntax, see documentation.
        script  Packets are defined in script files, that contain token based packets.
        pcap    pcap file of captured packets (e.g via wireshark or tcpdump) will be replayed.
    -r --raw
        Short for --input=raw
    -s --script
        Short for --input=script
    -p --pcap
        Short for --input=pcap
    -l N --loop=N
        Send all files/packets N times. Default: N = 1
    -d TIME --delay=TIME
        Packet transmission is delayed TIME.Resolution depends on -t parameter. Default is microseconds.
    -t RESOLUTION --resolution=RESOLUTION
        Resolution of transmission time. This affects -d parameter as well as all timestamps in script files
        Possible values are 'u'= microseconds (default), 'm'= milliseconds and 's'= seconds
    --interactive[=KEYLIST]
        Enable interactive mode (EXPERIMENTAL). In interactive mode no packets are sent automatically.
        Instead the packets are bound to keys and only sent when the corresponding key
        is pressed. The default implementation binds the first 10 packets to the keys 1, 2, ... 0.
    --write-to-pcap=OUTFILE
        Write generated packets to pcap file OUTFILE.
    --dissect
        Prints the dissected content of sent packets as known from tcpdump.

### Examples

Raw packet

    tcppump -i eth0 --raw 12345678abcdef12345678

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
