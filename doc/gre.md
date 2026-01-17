# Generic Routing Encapsulation (GRE)

## Protocol Specifier
```
gre
gre6
```
## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
gre | gre6(
    dmac        : MAC        [optional]
    smac        : MAC        [optional]
    dip         : IP4 | IP6
    sip         : IP4 | IP6  [optional]
    protocol    : INT16      [optional]
    key         : INT32      [optional]
    seq         : INT32      [optional]
    chksum      : INT16      [optional]
    payload     : BYTESTREAM [optional]
)
```

Note: Optionally all [VLAN tag parameters](doc/ethernet.md) and optional [IPv4](doc/ipv4.md) or [IPv6](doc/ipv6.md) parameters are allowed and can be combined with GRE fields.

## Parameter reference

- Name: `dmac`
    - Meaning: Destination EUI-48 MAC address
    - Type: MAC
    - Optional: yes (optional if `dip` is multicast)
    - Example: `dmac=12:23:34:34:44:44`

- Name: `dip`
    - Meaning: Destination IPv4 address (outer IP header)
    - Type: IPv4 or IPv6 (`gre6`) address
    - Example: `dip=1.2.3.4`

- Name: `smac`
    - Meaning: Source EUI-48 MAC address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `smac=aa:bb:cc:dd:ee:ff`

- Name: `sip`
    - Meaning: Source IPv4 address
    - Type: IPv4 or IPv6 (`gre6`) address
    - Optional: yes — Default: local interface IP (if available)
    - Example: `sip=192.168.0.1`

- Name: `protocol`
    - Meaning: Protocol type (GRE payload protocol)
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Example: `protocol=1234`

- Name: `key`
    - Meaning: GRE key field
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff
    - Optional: yes
    - Example: `key=1`

- Name: `seq`
    - Meaning: GRE sequence number
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff
    - Optional: yes
    - Example: `seq=10`

- Name: `chksum`
    - Meaning: GRE checksum (0 to auto-calc)
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Optional: yes — Behavior: If `0` checksum is calculated automatically; setting non-zero may be used to craft malformed packets.
    - Example: `chksum=0`

- Name: `payload`
    - Meaning: Encapsulated payload (bytestream or embedded packet)
    - Type: Bytestream / embedded packet
    - Optional: yes
    - Note: For payload formats (hex, quoted string, random, embedded packet) see `doc/PACKET_REFERENCE.md`.
    - Example: `payload=*`

## Standards
RFC2784, RFC2890


## Examples

minimum GRE packet with source-mac and ip taken from network interface
```
gre(dmac=12:23:34:34:44:44, dip=1.2.3.4, protocol=1234)
```

same GRE packet with embedded udp packet
```
gre(dmac=12:23:34:34:44:44, dip=1.2.3.4, protocol=1234, payload=<udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=*)>)
```

full blown GRE packet with random payload and automatic calculated checksum
```
gre(dmac=12:23:34:34:44:44, dip=1.2.3.4, protocol=1234, key=1, seq=10, chksum=0, payload=*)
```

full blown GRE packet with random payload and wrong checksum
```
gre(dmac=12:23:34:34:44:44, dip=1.2.3.4, protocol=1234, key=1, seq=10, chksum=44444, payload=*)
```