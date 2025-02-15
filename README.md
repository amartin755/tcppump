[![master: build-and-test](https://github.com/amartin755/tcppump/actions/workflows/build-and-test.yml/badge.svg?branch=master)](https://github.com/amartin755/tcppump/actions/workflows/build-and-test.yml)
[![devel: build-and-test](https://github.com/amartin755/tcppump/actions/workflows/build-and-test.yml/badge.svg?branch=devel)](https://github.com/amartin755/tcppump/actions/workflows/build-and-test.yml)

# tcppump
A Simple Ethernet Network Packet Generator

tcppump is a lightweight command-line tool for generating Ethernet packets, with a focus on low-level protocols.

Features
* OS-independent packet generation with full control over all protocol details, allowing for the creation of deliberately malformed packets.
* Packet definition using a token-based syntax.
* Packet transmission via command-line parameters or script-based execution.
* Time-triggered packet transmission (real-time mode). Note: Accuracy depends on the host environment.
* Looping functionality for repeated packet transmission.
* User-defined delays between packets.
* Real-time replay of PCAP files.
* Configurable source MAC and IP addresses (can use the network adapter’s addresses).
* Automatic ARP resolution for unknown hosts
* Random source and destination MAC addresses
* Output of generated network traffic to PCAP or text files

Supported protocols and packet formats
* Raw packets as byte streams
* Ethernet II packets
* IEEE 802.3 packets with IEEE 802.2 headers (LLC), including SNAP.
* VLAN tagging (including multiple tagging)
* ARP
* Raw IPv4
* Raw IPv6 (fragmentation not yet supported)
* UDP
* TCP
* VRRP
* STP/RSTP
* IGMP (v1/v2 only, v3 partially)
* ICMP
* VXLAN
* GRE

**Note: This tool is currently in the alpha stage. Interfaces and functionality are subject to change.**

## Build instructions
`tcppump` has been successfully compiled on **Ubuntu Linux (x64)** using **GCC** and on **Windows 10/11** using **MinGW32, MinGW64 (MSYS2), and MSVC.**
In theory, it should also compile with other **C++11-compatible** compilers, though this has not been tested.

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
        Usage: tcppump -i IFC [OPTIONS] packets|infiles
        tcppump -w OUTFILE [OPTIONS] packets|infiles

        -h, --help
                                Display this text
        --version
                                Show detailed version information
        -v, --verbose
                                Produce verbose output when parsing and printing. This option can be
                                supplied multiple times (up to 4 times, e.g., -vvvv) for even more debug
                                output.
        -i <IFC>, --interface <IFC>
                                Specify the name of the network interface through which packets are sent.
        --myip4 <IPV4>
                                Use the specified IPv4 address as the source IP address instead of the
                                network interface's IP address.
        --myip6 <IPV6>
                                Use the specified IPv6 address as the source IP address instead of the
                                network interface's IP address.
        --mymac <MAC>
                                Use the specified MAC address as the source MAC address instead of the
                                network interface's MAC address.
        --mtu <MTU>
                                Use the specified MTU instead of the network interface's MTU.
        --rand-smac
                                Use a random source MAC address. This overwrites --mymac and any explicitly
                                defined addresses in the packets.
        --rand-dmac
                                Use a random destination MAC address. This overwrites all explicitly
                                defined destination MAC addresses in the packets.
        --overwrite-dmac <MAC>
                                Overwrite the destination MAC address of all packets with the specified MAC
                                address.
        -s, --script
                                Read packets from script file instead of command-line.
        --pcap [SCALE]
                                Replay PCAP files of captured packets (e.g via Wireshark or tcpdump). The
                                transmission time can be scaled using the optional SCALE parameter. The
                                default value for SCALE is 1.0, meaning the file is played in real-time. A
                                value of 2.0 slows playback to half speed, while 0.5 plays it at twice the
                                speed. A value of 0 plays the file as quickly as possible.
        -l <N>, --loop <N>
                                Send all files/packets N times. Default: N = 1. If N = 0, packets will be
                                sent infinitely until Ctrl+c is pressed.
        -d <TIME>, --delay <TIME>
                                Delay the packet transmission by TIME. Resolution depends on the -t
                                parameter. By default, no delay is applied.
        -t <RESOLUTION>, --resolution <RESOLUTION>
                                Set the time resolution for packet transmission. This affects -d parameter
                                as well as all timestamps in script files. Possible values are 'u'=
                                microseconds, 'm'= milliseconds(default), 'c'= centiseconds and 's'=
                                seconds
        -w <OUTFILE>
                                Write raw packet data to OUTFILE, or to the standard output if OUTFILE is
                                set to '-'.
        -F <FORMAT>
                                Set the file format of the output capture file written using the -w option.
                                Supported formats are: 'pcap' (default), 'text', 'hexstream', 'hexdump'
        -a, --arp
                                Resolve the destination MAC address for IPv4 packets using ARP. If the
                                destination MAC address is omitted in IPv4 packets, it will be
                                automatically determined via ARP.
        --predictable-random
                                Use a simple sequence instead of random numbers to generate predictable
                                values.


### Packet Syntax
For details see [Packet Reference](PACKET_REFERENCE.md)
```
raw                 raw custom packet
 optional
   byte             Raw byte value
   be16             Big-endian 16-bit value
   be32             Big-endian 32-bit value
   be64             Big-endian 64-bit value
   le16             Little-endian 16-bit value
   le32             Little-endian 32-bit value
   le64             Little-endian 64-bit value
   ip4              IPv4 address
   ip6              IPv6 address
   mac              EUI-48 Mac address
   stream           Data stream

eth                 Ethernet II or IEEE802.3 packet
   dmac             Destination EUI-48 Mac address
   payload          Ethernet payload data
 optional
   smac             Source EUI-48 Mac address
   ethertype        EtherType field
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator
   dsap             IEEE 802.2 DSAP field
   ssap             IEEE 802.2 SSAP field
   control          Control field
   oui              Organizationally Unique Identifier
   protocol         Protocol identifier

arp                 Raw ARP packet
   op               Opcode, 1 = request, 2 = reply
   dip              Destination IP address
 optional
   smac             Source EUI-48 Mac address
   dmac             Destination EUI-48 Mac address
   sip              Source IP address
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

arp-probe           ARP probe packet
   dip              Destination IP address
 optional
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

arp-announce        ARP announce packet
 optional
   dip              Destination IP address
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

ipv4                Raw IPv4 packet
   dip              Destination IP address
   protocol         Transport layer protocol
   payload          IP packet payload
 optional
   smac             Source EUI-48 Mac address
   dmac             Destination EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

ipv6                Raw IPv6 packet
   dip              Destination IP address
   protocol         Transport layer protocol
   payload          IP packet payload
 optional
   smac             Source EUI-48 Mac address
   dmac             Destination EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   sip              Source IP address
   fl               IPv6 Flow Label
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

udp                 IPv4 User Datagram Protocol
   dip              Destination IP address
   sport            Source UDP port
   dport            Destination UDP port
 optional
   smac             Source EUI-48 Mac address
   dmac             Destination EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator
   payload          UDP packet payload
   chksum           UDP checksum

udp6                IPv6 User Datagram Protocol
   dip              Destination IP address
   sport            Source UDP port
   dport            Destination UDP port
 optional
   smac             Source EUI-48 Mac address
   dmac             Destination EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   sip              Source IP address
   fl               IPv6 Flow Label
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator
   payload          UDP packet payload
   chksum           UDP checksum

vrrp                Virual Router Redundancy Protocol V2
   vrip             Virtual Router IP address
   vrid             Virtual Router ID
 optional
   smac             Source EUI-48 Mac address
   sip              Source IP address
   vrprio           Virtual Router Priority
   type             VRRP message type
   aint             Advertisement Interval
   chksum           VRRP checksum
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

vrrp3               Virual Router Redundancy Protocol V3
   vrip             Virtual Router IP address
   vrid             Virtual Router ID
 optional
   smac             Source EUI-48 Mac address
   sip              Source IP address
   vrprio           Virtual Router Priority
   type             VRRP message type
   aint             Advertisement Interval
   chksum           VRRP checksum
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

stp                 Spanning Tree Protocol
 optional
   smac             Source EUI-48 Mac address
   rbprio           Root Bridge Priority
   rbidext          Root Bridge ID Extension
   rbmac            Root Bridge EUI-48 Mac address
   bprio            Bridge Priority
   bidext           Bridge ID Extension
   bmac             Bridge EUI-48 Mac address
   pprio            Port Priority
   pnum             Port Number
   msgage           Message Age
   maxage           Max Age
   hello            Hello Time
   delay            Forward Delay
   topochange       Topology Change
   topochangeack    Topology Change Acknowledgement
   rpathcost        Root Path Cost
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

rstp                Rapid Spanning Tree Protocol
 optional
   smac             Source EUI-48 Mac address
   rbprio           Root Bridge Priority
   rbidext          Root Bridge ID Extension
   rbmac            Root Bridge EUI-48 Mac address
   bprio            Bridge Priority
   bidext           Bridge ID Extension
   bmac             Bridge EUI-48 Mac address
   pprio            Port Priority
   pnum             Port Number
   msgage           Message Age
   maxage           Max Age
   hello            Hello Time
   delay            Forward Delay
   topochange       Topology Change
   topochangeack    Topology Change Acknowledgement
   rpathcost        Root Path Cost
   portrole         Port Role
   proposal         Proposal
   learning         Learning Mode
   forwarding       Forwarding Mode
   agreement        Agreement
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

stp-tcn             STP Topology Change Notification

igmp                Raw IGMP V1/V2 packet
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
   group            Multicast group address
   type             IGMP message type
 optional
   time             IGMP Time
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

igmp-query          IGMP V1/V2 Query
 optional
   time             IGMP Time
   group            Multicast group address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

igmp3-query         IGMP V3 Query
 optional
   time             IGMP Time
   group            Multicast group address
   s                Suppress Router-side Processing
   qrv              Query Response Interval
   qqic             Querier's Query Interval Count
   rsip             Router Source IP address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

igmp-report         IGMP V1/V2 Report
   group            Multicast group address
 optional
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

igmp-leave          IGMP V1/V2 Leave
   group            Multicast group address
 optional
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

icmp                Raw ICMPv4 packet
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
   type             ICMPv4 message type
   code             ICMPv4 message code
 optional
   payload          ICMPv4 message payload
   chksum           ICMPv4 checksum
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

icmp-unreachable    ICMPv4 Unreachable
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
 optional
   code             ICMPv4 message code
   payload          ICMPv4 message payload
   chksum           ICMPv4 checksum
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

icmp-src-quench     ICMPv4 Source Quench
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
 optional
   code             ICMPv4 message code
   payload          ICMPv4 message payload
   chksum           ICMPv4 checksum
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

icmp-time-exceeded  ICMPv4 Time Exceeded
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
 optional
   code             ICMPv4 message code
   payload          ICMPv4 message payload
   chksum           ICMPv4 checksum
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

icmp-redirect       ICMPv4 Redirect
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
   gw               Gateway address
 optional
   code             ICMPv4 message code
   payload          ICMPv4 message payload
   chksum           ICMPv4 checksum
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

icmp-echo           ICMPv4 Echo Request (Ping)
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
 optional
   id               ICMPv4 identifier
   seq              ICMPv4 sequence number
   payload          ICMPv4 message payload
   chksum           ICMPv4 checksum
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

icmp-echo-reply     ICMPv4 Echo Reply
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
 optional
   id               ICMPv4 identifier
   seq              ICMPv4 sequence number
   payload          ICMPv4 message payload
   chksum           ICMPv4 checksum
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

tcp                 Raw TCP packet
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
   sport            Source TCP port
   dport            Destination TCP port
   seq              TCP sequence number
   ack              TCP acknowledgment number
 optional
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   win              TCP window size
   urgptr           TCP urgent pointer
   FIN              TCP FIN flag
   SYN              TCP SYN flag
   RESET            TCP RESET flag
   PUSH             TCP PUSH flag
   ACK              TCP ACK flag
   URGENT           TCP URGENT flag
   ECN              TCP ECN flag
   CWR              TCP CWR flag
   NONCE            TCP nonce
   payload          TCP packet payload
   chksum           TCP checksum
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

vxlan               IPv4 Virtual eXtensible Local Area Network
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
   sport            Source UDP port
   dport            Destination UDP port
   vni              VXLAN Network Identifier
   payload          VXLAN payload data
 optional
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

vxlan6              IPv6 Virtual eXtensible Local Area Network
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
   sport            Source UDP port
   dport            Destination UDP port
   vni              VXLAN Network Identifier
   payload          VXLAN payload data
 optional
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   sip              Source IP address
   fl               IPv6 Flow Label
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

gre                 IPv4 Generic Routing Encapsulation
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
 optional
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   df               IPv4 Don't Fragment flag
   sip              Source IP address
   id               IPv4 packet identifier
   key              GRE key
   seq              GRE sequence number
   chksum           GRE checksum
   payload          GRE payload data
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator

gre6                IPv6 Generic Routing Encapsulation
   dmac             Destination EUI-48 Mac address
   dip              Destination IP address
 optional
   smac             Source EUI-48 Mac address
   dscp             Differentiated Services Code Point
   ecn              Explicit Congestion Notification
   ttl              Time To Live
   sip              Source IP address
   fl               IPv6 Flow Label
   key              GRE key
   seq              GRE sequence number
   chksum           GRE checksum
   payload          GRE payload data
   vid              VLAN Identifier
   vtype            VLAN Type
   prio             VLAN Priority
   dei              Drop Eligible Indicator
```


### Examples

Raw packet

    tcppump -i eth0 "raw(stream=1122334455667788990011223344)"

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
