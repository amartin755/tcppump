# Script file definition

The input file contains a list of ethernet packets in an human readable c-like syntax. Each packet definition has to be terminated with semicolon.

## Syntax
Optional parameters are market with []

    [timestamp:] protocol(parameters);

* `timestamp` relative time as integer followed by ':' (us since beginning of the file) e.g. 123456 if time starts with '+', time is relative to the previous packet; e.g. +100
* `protocol` protocol specifier , see PROTOCOLS
* `parameters` protocol parameters in parantheses as comma separated parameter-value pairs. e.g. (hugo=123, egon=456)

Comments start with '#'

Examples:

    # this is a comment
    +1234:   theProtocol(parameter-x = 3333333333333333333333333333333333333333);
    1000000: theProtocol(parameter-x = 3333333333333333333333333333333333333333);


## Protocol definitions

### Raw unstructured ethernet packet as byte stream
#### Protocol Specifier

    raw

#### Parameters
Payload in ascii hex

    payload

#### Example

    raw(payload = 112233445566aabbccddeeff08001234567890abcdef);


### Ethernet II or IEEE802.3 packet format
#### Protocol Specifier

    eth

#### Parameters
Destination EUI-48 MAC address

    dmac

Payload in ascii hex

    payload

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Ethertype or length of the packet (range 0 - 0xffff). If ommited or LLC header is defined, length will be calculated based on `payload`

    ethertype (optional)

__VLAN tags__ (IEEE 802.1Q)

For vlan tagged packets parameter 'vid' is mandatory, if ommitted all other vlan parameters will be ignored and an untagged packet is compiled.
Multiple vlan tags are also allowed. 'vid' must be the first parameter of a vlan tag.

VLAN id (TCI.VID) (range 0-4095)

    vid

VLAN prio (TCI.PCP)(range 0-7; default 0)

    prio (optional)

Drop Eligible Indicator (range 0-1; default 0)

    dei (optional)

VLAN type (1 = Customer VLAN, 2 = Provider VLAN; default 1)

    vtype (optional)

__LLC header__ (IEEE 802.2)

For packets with LLC header both parameters 'dsap' and 'ssap' have to be defined,

Destination Service Access point (range 0 - 0xff)

    dsap

Source Service Access Point (range 0 - 0xff)

    ssap

Control word (range 0 - 0xffff; default 3); will be compiled to a 8 bit value, if bit 0/1 == 1, otherwise compiled to 16 bit value

    control (optional)

__LLC header with SNAP extension__

LLC header parameters must not be defined for snap packets. Their content is filled automatically. Otherwise a normal LLC packet is compiled.
Organizationally Unique Identifier (range 0 - 0xffffff)

    oui

Protocol type (range 0 - 0xffff)

    protocol

#### Examples

    # IEEE802.3 packet
    eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, payload=1234567890abcdef);
    # Ethernet II packet
    eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, ethertype=0x0800, payload=1234567890abcdef);
    # VLAN tagged packet (default tag values)
    eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=1, ethertype=0x0800, payload=1234567890abcdef);
    # VLAN tagged packet
    eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=42, prio=3, ethertype=0x0800, payload=1234567890abcdef);
    # VLAN double tagged packet
    eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=100, vtype=2, vid=42, prio=3, ethertype=0x8123, payload=1234567890abcdef);
    # IEEE802.3 packet with LLC header
    eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, dsap = 0x12, ssap = 0x34, control = 0x11, payload = 1122);
    # IEEE802.3 packet with LLC header and VLAN tag
    eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vlan=1, vid=42, prio=3, dsap = 0x12, ssap = 0x34, control = 0x11, payload = 1122);
    # SNAP packet (IEEE802.3 packet with LLC header and SNAP extension)
    eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, oui = 0x808182, protocol = 0x34, payload = 1234567890abcdef);


### Address Resolution Protocol
#### Protocol Specifier

    arp
    arp-probe
    arp-announce

#### Parameters
Destination IPv4 address

    dip

Opcode (1 = request, 2 = reply, range 0 - 0xffff; default 1)

    op (optional)

Source EUI-48 MAC address; If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Destination EUI-48 MAC address (default 00:00:00:00:00:00)

    dmac (optional)

`arp-probe` and `arp-announce` accept only parameter `dip`.
Optionally all vlan tag parameters (see above) are also allowed.

#### Examples

    # full crafted arp packet
    arp(op=1, smac=10:22:33:44:55:66, sip=192.168.0.166, dmac=01:02:03:04:05:06, dip=1.2.3.4);
    # who has 11.22.33.44, tell IP of host
    arp(dip=11.22.33.44);
    # arp-probe as specified in rfc 5227; same as "who has" above, but sender IP set to zero
    arp-probe(dip=1.2.3.4); # short for arp(op=1, sip=0.0.0.0, dmac=00:00:00:00:00:00, dip=1.2.3.4)
    # arp-announce as specified in rfc 5227; so-called gratitious arp
    arp-announce(dip=1.2.3.4); # short for arp(op=1, sip=1.2.3.4, dmac=00:00:00:00:00:00, dip=1.2.3.4)

### IPv4
#### Protocol Specifier

    ipv4

#### Parameters
Destination EUI-48 MAC address. Note: Optional if `dip` is a multicast address

    dmac (optional if dip is multicast)

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Destination IPv4 address

    dip

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Differentiated services code point (range 0 - 0x3f; default 0)

    dscp (optional)

Explicit congestion notification (range 0 - 3; default 0)

    ecn (optional)

Time to life (range 0 - 255; default 64)

    ttl (optional)

Don't fragment (default 0; 1 = don't fragment)

    df (optional)

Protocol number (range 0 - 255) see https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml

    protocol

Payload in ascii hex

    payload

Optionally all vlan tag parameters (see above) are also allowed.

#### Examples

    # raw simple ipv4 packet
    ipv4(dmac = 11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678);
    # the same packet combined with vlan tag
    ipv4(vid=42, dmac = 11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678);
    # raw fully defined IPv4 packet
    ipv4(smac=80:12:34:45:67:89, dmac = 11:22:33:44:55:66, sip=192.168.0.1, dip=172.16.1.2, ttl=200, dscp=16, ecn=1, df=1, protocol=254, payload=12345678);
    # raw simple ipv4 packet with multicast destination (dmac is therefore set automatically)
    ipv4(dip=224.2.3.4, protocol=254, payload=12345678);
    # same as above, but dmac is forced to 11:22:33:44:55:66
    ipv4(dmac = 11:22:33:44:55:66, dip=224.2.3.4, protocol=254, payload=12345678);


### IGMP
#### Protocol Specifier (raw IGMPv1/v2 packet)

    igmp

#### Parameters
Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

IGMP Message Type (range 0 - 255, default 0x11)

    type

Max. Respnd Time in 1/10 seconds (range: 0 - 255, default 0)

    time (optional)

Group IPv4 Address. If ommited, value of parameter `dip` is used.

    group (optional)

#### Protocol Specifier (IGMPv2 General/Group-Specific Membership Query)

    igmp-query

#### Parameters
Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Max. Respnd Time in 1/10 seconds (range: 0 - 255, default 100 = 10 sec.)

    time (optional)

Group IPv4 Address. If ommited --> General-Membership-Query

    group (optional)

#### Protocol Specifier (IGMPv2 Membership Report / IGMPv2 Leave Group)

    igmp-report
    igmp-leave

#### Parameters
Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Group IPv4 Address.

    group

#### Protocol Specifier (IGMPv3 General/Group-Specific/Group-and-Source-Specific Membership Query)

    igmp3-query

#### Parameters
Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Max. Respnd Time in seconds (range: 0 - 3174.4, default 10)

    time (optional)

S Flag (Suppress Router-Side Processing) (range: 0 - 1, default 0)

    s (optional)

Querier's Robustness Variable (range: 0 - 7, default 2) Note: According to RFC3376 zero is an invalid value

    qrv (optional)

Querier's Query Interval Code in seconds (range: 0 - 31744, default 125)

    qqic (optional)

Group IPv4 Address. If ommited --> General-Membership-Query

    group (optional)

Router Source IPv4 Address; Zero to 366 IP addresses are allowed. If you provide more addresses they will be silently ignored.

    rsip (optional)

#### Protocol Specifier (IGMPv3 Membership Report)

    igmp3-report

#### Parameters
Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

IP multicast address to which this Group Record pertains.

    multicast (optional)

Record Type

    type (optional)

Router Source IPv4 Address; Zero to 366 IP addresses are allowed. If you provide more addresses they will be silently ignored.

    rsip (optional)

#### Examples

    # TODO


### UDP
#### Protocol Specifier

    udp

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Source port (range 0 - 0xffff)

    sport

Destination port (range 0 - 0xffff)

    dport

Checksum (range 0 - 0xffff) If ommited, checksum is calculated automatically. Setting the checksum manually is only useful to force creation of malformed packets.

    chksum (optional)

Payload in ascii hex

    payload (optional)

Optionally all vlan tag parameters and all optional ipv4 parameters (see above) are also allowed.

#### Examples

    # UDP packet with source-mac and ip taken from network interface
    udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=12345678);
    # UDP packet with explicit source-mac and ip
    udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, smac=80:12:34:45:67:89, sip=192.168.0.1, sport=1234, dport=2345, payload=12345678);
    # UDP packet with to multicast destination and source ip taken from network interface
    udp(dip=224.2.3.4, sport=1234, dport=2345, payload=12345678);

### VRRP Virtual Router Redundancy Protocol
Supported are the protocol versions 2 according to RFC3768 (vrrp) and version 3 according to RFC5798 (vrrp3).

#### Protocol Specifier

    vrrp
    vrrp3

#### Parameters
Source EUI-48 MAC Address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 Address. If ommited, address of the network interface is used

    sip (optional)

Virtual Router ID (range 1 - 255)

    vrid

Virtual Router IPv4 Address; Up to 255 IP addresses are allowed. At least one IP is mandatory, if you provide more addresses they will be silently ignored.

    vrip

Virtual Router Priority (range 0 - 255, default 100)

    prio (optional)

Advertisement Interval. Note: Value range and unit depends on the specified protocol version. V2: seconds (range: 0 - 255, default 1), V3: centiseconds (range: 0 - 4095, default 100)

    aint (optional)

VRRP Packet Type (range 0 - 15; default 1). Note: According to RFC3768 only 1 is a valid value

    type (optional)

Checksum (range 0 - 0xffff). If ommited, checksum is calculated automatically

    chksum (optional)


NOTE: Optionally all vlan tag parameters and all optional ipv4 parameters (see above) are also allowed.

#### Examples

    # VRRP V3 packet with default values; virtual router with id 42 and ip address 1.2.3.4
    # Source-mac and ip are taken from network interface
    vrrp3(vrid=42, vrip=1.2.3.4);
    # The same as above with protocol version 2
    vrrp(vrid=42, vrip=1.2.3.4);
    # full defined VRRP2 packet with two virtual router ip addresses
    vrrp(smac=80:12:34:45:67:89, sip=192.168.0.1, vrid=42, prio=120, vrip=1.2.3.4, vrip=1.2.3.5, aint=3);
    # malformed vrrp packet (undefined type)
    vrrp(vrid=42, vrip=1.2.3.4, type=3);
    # malformed vrrp packet (wrong checksum)
    vrrp(vrid=42, vrip=1.2.3.4, chksum=0x4321);


### Spanning Tree Protocol family (STP, RSTP)
IEEE802.1D-2004

#### Protocol Specifier (Configuration BPDU)

    stp
    rstp

#### Parameters
Source EUI-48 MAC Address. If ommited, address of the network interface is used

    smac (optional)

Root Bridge Priority (range 0 - 15, default 0)

    rbprio (optional)

Root Bridge System ID Extension (range 0 - 4095, default 0)

    rbidext (optional)

Root Bridge EUI-48 MAC Address. If ommited, address of the network interface is used

    rbmac (optional)

Root Path Cost (STP: range 1 - 65535, default 4; RSTP: range 1 - 4294967295, default 20000)

    rpathcost (optional)

Bridge Priority (range 0 - 15, default 8)

    bprio (optional)

Bridge System ID Extension (range 0 - 4095, default 0)

    bidext (optional)

Bridge EUI-48 MAC Address. If ommited, address of the network interface is used

    bmac (optional)

Port Priority (range 0 - 15, default 8)

    pprio (optional)

Port Number (range 1 - 4095, default 1)

    pnum (optional)

Message Age (seconds)(range 0.0 - 255.996, default 0)

    msgage (optional)

Max Age in seconds (range 0.0 - 255.996, default 20) Note: According to IEEE802.1D-2004 only range 6 - 40 is allowed

    maxage (optional)

Hello Time (seconds)(range 0.0 - 255.996, default 2)

    hello (optional)

Forward Delay (seconds)(range 0.0 - 255.996, default 15)  Note: According to IEEE802.1D-2004 only range 4 - 30 is allowed

    delay (optional)

Topology Change Flag(default 0; 1 = Topology Change)

    topochange (optional)

Topology Change Acknowledgement Flag(default 0; 1 = Topology Change Acknowledgement)

    topochangeack (optional)

RSTP only: Port Role (range 1 - 3, default 3; 1 = Alternate or Backup, 2 = Root, 3 = Designated)

    portrole (optional)

RSTP only: Proposal Flag (default 0; 1 = proposal)

    proposal (optional)

RSTP only: Learning Flag (default 1; 0 = no learning, 1 = learning)

    learning (optional)

RSTP only: Forwarding Flag (default 1; 0 = no forwarding, 1 = forwarding)

    forwarding (optional)

RSTP only: Agreement Flag (default 0; 1 = agreement)

    agreement (optional)

#### Examples

    # STP config BPDU with default parameters. MAC address of network interface is used as
    # source address, bridge address and root bridge address
    stp();

#### Protocol Specifier (TCN BPDU)

    stp-tcn

#### Parameters
This packet has no parameters.
