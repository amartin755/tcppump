# Packet Syntax Reference

tcppump's input consists of a list of Ethernet packets, defined using a human-readable C-like syntax. The list can either be passed directly as a command-line parameter or included in a script file (using the `-s` or `--script` parameter).

## Packet
### Abstract Format
Each packet is defined as follows (optional parameters are marked with brackets `[]`):

    [timestamp:] protocol([parameter_list])

* `timestamp`: A relative time value, specified as an integer followed by `:` (relative to the start of tcppump), e.g., `123456`. If the time starts with `+`, it is relative to the previous packet, e.g., `+100`.
* `protocol`: A protocol specifier (see *Protocol Definitions*).
* `parameter_list`: Protocol parameters enclosed in parentheses, provided as comma-separated parameter-value pairs: `parameter=value, parameter=value, ...`. Example: `(hugo=123, egon=456)`.

**Note:** All identifiers are case-sensitive.

### Parameters
Each protocol defines the names and types of its parameters (see *PROTOCOLS*). Depending on their type, parameter values can be:

* **Integer:** Decimal (`1234`), hexadecimal (`0x1234`), random number (`*`) or range restricted random number (`*[1-4]` `*[0x10-0x13]`).
* **Float:** Example: `1.2`
* **Bytestream:** A sequence of 8-bit values (bytes or octets), which can be defined as:

  - **ASCII Hex Values:** Each byte is represented as an ASCII-hex value (e.g., `01020304ABCD`).
  - **String:** A sequence of printable ASCII characters enclosed in double quotes (e.g., `"Hello World"`).
  - **Random:** `*` represents a random sequence of 32 bytes. A specific length can be defined by appending the desired byte length after the asterisk. For example, `*16` generates a random 16-byte sequence.

* **MAC Address:** A EUI-48 MAC address as six colon separated hexadecimal numbers (e.g., `12:23:34:45:56:67`). The entire MAC address or it components can also be random (`*`) or range restricted random (`*[80-8a]`)
* **IPv4 Address:** Example: `1.2.3.4`.  The whole address as well it's components can also be random (examples: `*`, `192.168.10.*` `192.168.*.*[100-2000]`)
* **Embedded Packet:** A fully defined packet can be embedded within another packet. The definition is enclosed in angle brackets `< >`, e.g., `<eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, payload=*)>`.


### Examples

    +1234:   protoMickey(color = 10, index = 0x16, msg = "Hello")

    1000000: protoMouse(payload = *32)

    donald(parZ = valueZ)

    doit()

    +1234:   protoMickey(color = 10, index = 0x16, msg = <protoMouse(payload = *32)>)

## Script files
Script files contain a list of packets as specified above. Each packet definition must be terminated with a semicolon (`;`). Comments start with `#`.

Example:

    # this is a comment
    +1234:   protoMickey(color = 10, index = 0x16, msg = "Hello");
    1000000: protoMouse(payload = *32);
    donald(parZ = valueZ);  # another comment
    doit();


## Protocol Definitions

### Full configurable raw Ethernet packet consisting of standard data types
For the definition of "normal" Ethernet packets the protocol `eth` is more suitable. The `raw` protocol is a powerful tool when it is used embedded into other protocols. For example it can be used to create the payload of an UDP packet.

#### Protocol Specifier

    raw

#### Parameters
All parameters are optional and can be used multiple times. The packet is compiled by adding parameter by parameter to the payload and therefore the order of the parameter is important.

Bytestream

    stream

Byte (integer: range 0 - 255)

    byte

16 bit big endian integer (range 0 - 65535)

    be16

16 bit little endian integer (range 0 - 65535)

    le16

32 bit big endian integer (range 0 - 4294967295)

    be32

32 bit little endian integer (range 0 - 4294967295)

    le32

64 bit big endian integer (range 0 - 18446744073709551615)

    be64

64 bit little endian integer (range 0 - 18446744073709551615)

    le64

IPv4 address

    ip4

IPv6 address

    ip6

EUI-48 MAC address

    mac

#### Examples

    raw(stream=112233445566aabbccddeeff08001234567890abcdef);
    raw(stream="Hello");
    raw(stream=*64);

    # demo of all available data types
    raw(byte=0x55, be16=0x1234, le16=0x1234, be32=0x11223344, le32=0x11223344,
        be64=0x0123456789abcdef, le64=0x0123456789abcdef, ip4=1.2.3.4,
        ip6=1002:3004:5006:7008:900A:B00C:D00E:F001, mac=10:20:30:40:50:60,
        stream="Hello World", byte=0xaa);

    # hand craftet ethernet packet. This is identical to
    #  eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, ethertype=0x0800, payload=1234567890abcdef)
    raw(mac=11:22:33:44:55:66, mac=aa:bb:cc:dd:ee:ff, be16=0x0800, stream=1234567890abcdef);


### Ethernet II or IEEE802.3 packet format
#### Protocol Specifier

    eth

#### Parameters
Destination EUI-48 MAC address

    dmac

Payload (bytestream)

    payload

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Ethertype or length of the packet (integer: range 0 - 0xffff). If ommited or LLC header is defined, length will be calculated based on `payload`

    ethertype (optional)

__VLAN tags__ (IEEE 802.1Q)

For vlan tagged packets parameter 'vid' is mandatory, if ommitted all other vlan parameters will be ignored and an untagged packet is compiled.
Multiple vlan tags are also allowed. 'vid' must be the first parameter of a vlan tag.

VLAN id (TCI.VID) (integer: range 0-4095)

    vid

VLAN prio (TCI.PCP)(integer: range 0-7; default 0)

    prio (optional)

Drop Eligible Indicator (integer: range 0-1; default 0)

    dei (optional)

VLAN type (integer: 1 = Customer VLAN, 2 = Provider VLAN; default 1)

    vtype (optional)

__LLC header__ (IEEE 802.2)

For packets with LLC header both parameters 'dsap' and 'ssap' have to be defined,

Destination Service Access point (integer: range 0 - 0xff)

    dsap

Source Service Access Point (integer: range 0 - 0xff)

    ssap

Control word (integer: range 0 - 0xffff; default 3); will be compiled to a 8 bit value, if bit 0/1 == 1, otherwise compiled to 16 bit value

    control (optional)

__LLC header with SNAP extension__

LLC header parameters must not be defined for snap packets. Their content is filled automatically. Otherwise a normal LLC packet is compiled.
Organizationally Unique Identifier (integer: range 0 - 0xffffff)

    oui

Protocol type (integer: range 0 - 0xffff)

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

Opcode (integer: 1 = request, 2 = reply, range 0 - 0xffff; default 1)

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

Differentiated services code point (integer: range 0 - 0x3f; default 0)

    dscp (optional)

Explicit congestion notification (integer: range 0 - 3; default 0)

    ecn (optional)

Time to life (integer: range 0 - 255; default 64)

    ttl (optional)

Don't fragment (integer: 1 = don't fragment; default 0)

    df (optional)

Identification (integer: range 0 - 0xffff; default 0 or auto value for fragmented packets)

    id (optional)

Protocol number (integer: range 0 - 255) see https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml

    protocol

Payload (bytestream)

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

Destination EUI-48 MAC address. Note: Optional if `dip` is a multicast address

    dmac (optional if dip is multicast)

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

IGMP Message Type (integer: range 0 - 255; default 0x11)

    type

V1: unused, V2: Max. Respnd Time in 1/10 seconds (integer: range 0 - 255; default 0)

    time (optional)

Checksum (integer: range 0 - 0xffff) If ommited, checksum is calculated automatically. Setting the checksum manually is only useful to force creation of malformed packets.

    chksum (optional)  NOT YET IMPLEMENTED

Group IPv4 Address.

    group

Additional payload (bytestream). Can be used to encode IGMPv0 Access Keys or to create malformed packets.

    payload (optional) NOT YET IMPLEMENTED

#### Protocol Specifier (IGMPv2 General/Group-Specific Membership Query)

    igmp-query

#### Parameters
Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Max. Respnd Time in 1/10 seconds (integer: range: 0 - 255; default 100 = 10 sec.)

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

Max. Respnd Time in seconds (float: range 0 - 3174.4; default 10)

    time (optional)

S Flag (Suppress Router-Side Processing) (integer: range 0 - 1; default 0)

    s (optional)

Querier's Robustness Variable (integer: range 0 - 7; default 2) Note: According to RFC3376 zero is an invalid value

    qrv (optional)

Querier's Query Interval Code in seconds (integer: range 0 - 31744; default 125)

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

Source port (integer: range 0 - 0xffff)

    sport

Destination port (integer: range 0 - 0xffff)

    dport

Checksum (integer: range 0 - 0xffff) If ommited, checksum is calculated automatically. Setting the checksum manually is only useful to force creation of malformed packets.

    chksum (optional)

Payload (bytestream)

    payload (optional)

Optionally all vlan tag parameters and all optional ipv4 parameters (see above) are also allowed.

#### Examples

    # UDP packet with source-mac and ip taken from network interface
    udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=12345678);
    # UDP packet with explicit source-mac and ip
    udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, smac=80:12:34:45:67:89, sip=192.168.0.1, sport=1234, dport=2345, payload=12345678);
    # UDP packet with to multicast destination and source ip taken from network interface
    udp(dip=224.2.3.4, sport=1234, dport=2345, payload=12345678);
    # UDP packet without payload
    udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345);
    # UDP packet with random payload
    udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=*);
    # UDP packet embedded structured payload
    udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345,
        payload=<raw(byte=0x55, be16=0x1234, le16=0x1234, be32=0x11223344)>);

### TCP
#### Protocol Specifier

    tcp

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address. Multicast addresses are also possible, which can be used for creation of malformed packets.

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Source port (integer: range 0 - 0xffff)

    sport

Destination port (integer: range 0 - 0xffff)

    dport

Sequence number (integer: range 0 - 0xffffffff)

    seq

Acknowledgement number (integer: range 0 - 0xffffffff)

    ack

Window size (integer: range 0 - 65535; default 1024)

    win (optional)

Urgent pointer (integer: range 0 - 65535; default 0)

    urgptr (optional)

Fin control flag (integer: range 0 - 1; default 0)

    FIN (optional)

Syn control flag (integer: range 0 - 1; default 0)

    SYN (optional)

Reset control flag (integer: range 0 - 1; default 0)

    RESET (optional)

Push control flag (integer: range 0 - 1; default 0)

    PUSH (optional)

Acknowledge control flag (integer: range 0 - 1; default 0)

    ACK (optional)

Urgent control flag (integer: range 0 - 1; default 0)

    URGENT (optional)

ECN-echo control flag (integer: range 0 - 1; default 0)

    ECN (optional)

Congestion-Window-Reduced control flag (integer: range 0 - 1; default 0)

    CWR (optional)

Nonce control flag (integer: range 0 - 1; default 0)

    NONCE (optional)

Checksum (integer: range 0 - 0xffff) If ommited, checksum is calculated automatically. Setting the checksum manually is only useful to force creation of malformed packets.

    chksum (optional)

Payload (bytestream)

    payload (optional)

Optionally all vlan tag parameters and all optional ipv4 parameters (see above) are also allowed.

#### Examples
    # TODO


### ICMP
RFC792
#### Protocol Specifier (raw ICMP packet)

    icmp

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

ICMP type (integer: range 0 - 255)

    type

Code (integer: range 0 - 255)

    code

Checksum (integer: range 0 - 0xffff) If ommited, checksum is calculated automatically. Setting the checksum manually is only useful to force creation of malformed packets.

    chksum (optional)

Payload (bytestream)

    payload (optional)

#### Protocol Specifier (ICMPv4 Unreachable)

    icmp-unreachable

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Code (integer: 0 = network, 1 = host, 2 = protocol, 3 = port, range 0 - 255, default 0)

    code (optional)

Payload (bytestream)

    payload (optional)

#### Protocol Specifier (ICMPv4 Source Quench)

    icmp-src-quench

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Code (integer: range 0 - 255, default 0)

    code (optional)

Payload (bytestream)

    payload (optional)

#### Protocol Specifier (ICMPv4 Time Exceeded)

    icmp-time-exceeded

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Code (integer: 0 = Time to Live exceeded in transit, 1 = Fragment reassembly time exceeded, range 0 - 255, default 0)

    code (optional)

Payload (bytestream)

    payload (optional)

#### Protocol Specifier (ICMPv4 Redirect)

    icmp-redirect

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Gateway IPv4 address

    gw

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Code (integer: 0 = network, 1 = host, 2 = protocol, default 0)

    code (optional)

Payload (bytestream)

    payload (optional)

#### Protocol Specifier (ICMPv4 Echo Request)

    icmp-echo

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Identifier (integer: range 0 - 65535, default 0)

    id (optional)

Sequence number (integer: range 0 - 65535, default 0)

    seq (optional)

Payload (bytestream)

    payload (optional)

#### Protocol Specifier (ICMPv4 Echo Reply)

    icmp-echo-reply

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Identifier (integer: range 0 - 65535, default 0)

    id (optional)

Sequence number (integer: range 0 - 65535, default 0)

    seq (optional)

Payload (bytestream)

    payload (optional)

Optionally all vlan tag parameters and all optional ipv4 parameters (see above) are also allowed.

#### Examples

    # TODO

### Virtual Router Redundancy Protocol (VRRP)
Supported are the protocol versions 2 according to RFC3768 (vrrp) and version 3 according to RFC5798 (vrrp3).

#### Protocol Specifier

    vrrp
    vrrp3

#### Parameters
Source EUI-48 MAC Address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 Address. If ommited, address of the network interface is used

    sip (optional)

Virtual Router ID (integer: range 1 - 255)

    vrid

Virtual Router IPv4 Address; Up to 255 IP addresses are allowed at least one IP is mandatory.

    vrip

Virtual Router Priority (integer: range 0 - 255; default 100)

    vrprio (optional)

Advertisement Interval. Note: Value range and unit depends on the specified protocol version. V2: seconds (range: 0 - 255, default 1), V3: centiseconds (integer: range 0 - 4095; default 100)

    aint (optional)

VRRP Packet Type (integer: range 0 - 15; default 1). Note: According to RFC3768 only 1 is a valid value

    type (optional)

Checksum (integer: range 0 - 0xffff). If ommited, checksum is calculated automatically

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

Root Bridge Priority (integer: range 0 - 15; default 0)

    rbprio (optional)

Root Bridge System ID Extension (integer: range 0 - 4095; default 0)

    rbidext (optional)

Root Bridge EUI-48 MAC Address. If ommited, address of the network interface is used

    rbmac (optional)

Root Path Cost (integer: STP: range 1 - 65535; default 4; RSTP: range 1 - 4294967295; default 20000)

    rpathcost (optional)

Bridge Priority (integer: range 0 - 15; default 8)

    bprio (optional)

Bridge System ID Extension (integer: range 0 - 4095; default 0)

    bidext (optional)

Bridge EUI-48 MAC Address. If ommited, address of the network interface is used

    bmac (optional)

Port Priority (integer: range 0 - 15; default 8)

    pprio (optional)

Port Number (integer: range 1 - 4095; default 1)

    pnum (optional)

Message Age (seconds)(float: range 0.0 - 255.996; default 0)

    msgage (optional)

Max Age in seconds (float: range 0.0 - 255.996; default 20) Note: According to IEEE802.1D-2004 only range 6 - 40 is allowed

    maxage (optional)

Hello Time (seconds)(float: range 0.0 - 255.996; default 2)

    hello (optional)

Forward Delay (seconds)(float: range 0.0 - 255.996; default 15)  Note: According to IEEE802.1D-2004 only range 4 - 30 is allowed

    delay (optional)

Topology Change Flag(integer: default 0; 1 = Topology Change)

    topochange (optional)

Topology Change Acknowledgement Flag(integer: default 0; 1 = Topology Change Acknowledgement)

    topochangeack (optional)

RSTP only: Port Role (integer: range 1 - 3, default 3; 1 = Alternate or Backup, 2 = Root, 3 = Designated)

    portrole (optional)

RSTP only: Proposal Flag (integer: default 0; 1 = proposal)

    proposal (optional)

RSTP only: Learning Flag (integer: default 1; 0 = no learning, 1 = learning)

    learning (optional)

RSTP only: Forwarding Flag (integer: default 1; 0 = no forwarding, 1 = forwarding)

    forwarding (optional)

RSTP only: Agreement Flag (integer: default 0; 1 = agreement)

    agreement (optional)

#### Examples

    # STP config BPDU with default parameters. MAC address of network interface is used as
    # source address, bridge address and root bridge address
    stp();

#### Protocol Specifier (TCN BPDU)

    stp-tcn

#### Parameters
This packet has no parameters.

### Virtual eXtensible Local Area Network (VXLAN)
RFC7348
#### Protocol Specifier

    vxlan

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Source port (integer: range 0 - 0xffff)

    sport

Destination port (integer: range 0 - 0xffff; default 4789)

    dport (optional)

VXLAN Network Identifier (integer: range 0 - 0xffffff; default 0)

    vni (optional)

Payload (bytestream or embedded packet)

    payload (optional)

Optionally all vlan tag parameters and all optional ipv4 parameters (see above) are also allowed.

#### Examples

    # VXLAN packet with source-mac and ip taken from network interface
    vxlan(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, vni=42, payload=1234567812345678);
    # same VXLAN packet with embedded Ethernet II packet
    vxlan(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, vni=42, payload=<eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, ethertype=0x8123, payload=1234567890abcdef);>);

### Generic Routing Encapsulation (GRE)
RFC2784, RFC2890
#### Protocol Specifier

    gre

#### Parameters
Destination EUI-48 MAC address. Note: If `dip` is a multicast address `dmac` will be set automatically.

    dmac (optional if dip is multicast)

Destination IPv4 address

    dip

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

Protocol type (integer: range 0 - 0xffff)

    protocol

Key field (integer: range 0 - 0xffffffff)

    key (optional)

Sequence number (integer: range 0 - 0xffffffff)

    seq (optional)

Checksum (integer: range 0 - 0xffff) If zero, checksum is calculated automatically. Setting the checksum manually is only useful to force creation of malformed packets.

    chksum (optional)

Payload (bytestream or embedded packet)

    payload (optional)

Optionally all vlan tag parameters and all optional ipv4 parameters (see above) are also allowed.

#### Examples

    # minimum GRE packet with source-mac and ip taken from network interface
    gre(dmac=12:23:34:34:44:44, dip=1.2.3.4, protocol=1234);
    # same GRE packet with embedded udp packet
    gre(dmac=12:23:34:34:44:44, dip=1.2.3.4, protocol=1234, payload=<udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=*)>);
    # full blown GRE packet with random payload and automatic calculated checksum
    gre(dmac=12:23:34:34:44:44, dip=1.2.3.4, protocol=1234, key=1, seq=10, chksum=0, payload=*);
    # full blown GRE packet with random payload and wrong checksum
    gre(dmac=12:23:34:34:44:44, dip=1.2.3.4, protocol=1234, key=1, seq=10, chksum=44444, payload=*);

