# Script file definition

The input file contains a list of ethernet packets in an human readable c-like syntax. Each packet definition has to be terminated with semicolon.

## Syntax
Optional parameters are market with []

    [timestamp:] protocol(parameters);

* `timestamp` relative time as integer followed by ':' (us since beginning of the file) e.g. 123456 if time starts with '+', time is relative to the previous packet; e.g. +100
* `protocol` protocol specifier , see PROTOCOLS
* `parameters` protocol parameters in parantheses as comma separated parameter-value pairs. e.g. (hugo=123, egon=456)
  comments start with #

examples:

    +1234:   theProtocol(parameter-x = 3333333333333333333333333333333333333333);
    1000000: theProtocol(parameter-x = 3333333333333333333333333333333333333333);


## Protocol definitions

### Raw unstructured ethernet packet as byte stream
#### protocol specifier

    raw

#### parameters
Payload in ascii hex

    payload

#### example

    raw(payload = 112233445566aabbccddeeff08001234567890abcdef);


### Ethernet II or IEEE802.3 packet format
#### protocol specifier

    eth

#### parameters
Destination EUI-48 MAC address

    dmac

Payload in ascii hex

    payload

Source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

Ethertype or length of the packet (range 0 - 0xffff). If ommited or LLC header is defined, length will be calculated based on `payload`

    ethertype (optional)

__VLAN tags__

note: for vlan tagged packets parameter .vid is mandatory, if ommitted all other vlan parameters will be ignored and a untagged packet is compiled

VLAN id (TCI.VID) (range 0-4095)

    vid

VLAN prio (TCI.PCP)(range 0-7; default 0)

    prio (optional)

Drop Eligible Indicator (range 0-1; default 0)

    dei (optional)

VLAN type (1 = Customer VLAN, 2 = Provider VLAN; default 1)

    vtype (optional)

__LLC header__

note: for packets with llc header both parameters .dsap and .ssap have to be defined

Destination Service Access point (range 0 - 0xff)

    dsap

Source Service Access Point (range 0 - 0xff)

    ssap

Control word (range 0 - 0xffff; default 3); will be compiled to a 8 bit value, if bit 0/1 == 1, otherwise compiled to 16 bit value

    control (optional)

__LLC header with SNAP extension__

Organizationally Unique Identifier (range 0 - 0xffffff)

    oui

Protocol type (range 0 - 0xffff)

    protocol

#### examples

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
#### protocol specifier

    arp

#### parameters
destination IPv4 address

    dip

opcode (1 = request, 2 = reply, whole 16bit range allowed; default 1)

    op (optional)

source EUI-48 MAC address; If ommited, address of the network interface is used

    smac (optional)

source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

destination EUI-48 MAC address (default 00:00:00:00:00:00)

    dmac (optional)

NOTE: Optionally all vlan tag parameters (see above) are also allowed.

#### examples

    # full crafted arp packet
    arp(op=1, smac=10:22:33:44:55:66, sip=192.168.0.166, dmac=01:02:03:04:05:06, dip=1.2.3.4);
    # who has 11.22.33.44, tell IP of host
    arp(dip=11.22.33.44);
    # arp-probe as specified in rfc 5227; same as "who has" above, but sender IP set to zero
    arp-probe(dip=1.2.3.4); # short for arp: .op=1 .sip=0.0.0.0 .dmac=00:00:00:00:00:00 .dip=1.2.3.4
    # arp-announce as specified in rfc 5227; so-called gratitious arp
    arp-announce(dip=1.2.3.4); # short for arp: .op=1 .sip=1.2.3.4 .dmac=00:00:00:00:00:00 .dip=1.2.3.4

### IPv4
#### protocol specifier

    ipv4

#### parameters
destination EUI-48 MAC address

    dmac

source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

destination IPv4 address

    dip

source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

differentiated services code point (range 0 - 0x1f; default 0)

    dscp (optional)

explicit congestion notification (range 0 - 3; default 0)

    ecn (optional)

time to life (range 0 - 255; default 64)

    ttl (optional)

don't fragment (default 0; 1 = don't fragment)

    df (optional)

protocol number (range 0 - 255) see https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml

    protocol

payload in ascii hex

    payload

NOTE: Optionally all vlan tag parameters (see above) are also allowed.

#### examples

    # raw simple ipv4 packet
    ipv4(dmac = 11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678);
    # the same packet combined with vlan tag
    ipv4(vid=42, dmac = 11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678);
    # raw fully defined IPv4 packet
    ipv4(smac=80:12:34:45:67:89, dmac = 11:22:33:44:55:66, sip=192.168.0.1, dip=172.16.1.2, ttl=200, dscp=16, ecn=1, df=1, protocol=254, payload=12345678);


### UDP
#### protocol specifier

    udp

#### parameters
destination EUI-48 MAC address

    dmac

destination IPv4 address

    dip

source EUI-48 MAC address. If ommited, address of the network interface is used

    smac (optional)

source IPv4 address; If ommited, address of the network interface is used

    sip (optional)

source port (range 0 - 0xffff)

    sport

destination port (range 0 - 0xffff)

    dport

checksum (range 0 - 0xffff) If ommited, checksum will be automatically calculated

    checksum (optional)

payload in ascii hex

    payload

NOTE: Optionally all vlan tag parameters and all optional ipv4 parameters (see above) are also allowed.

#### examples

TODO
