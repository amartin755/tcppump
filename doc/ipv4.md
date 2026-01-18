# Raw IPv4 packet

## Protocol Specifier
```
ipv4
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
ipv4(
    dmac        : MAC        [optional]
    smac        : MAC        [optional]
    dip         : IP4
    sip         : IP4        [optional]
    dscp        : INT        [optional]
    ecn         : INT        [optional]
    ttl         : INT8       [optional]
    df          : BIT        [optional]
    id          : INT16      [optional]
    protocol    : INT8
    hchksum     : INT16      [optional]
    payload     : BYTESTREAM
)
```
Note: [VLAN tag parameters](doc/ethernet.md) may be specified optionally.

## Parameter reference

- Name: `dmac`
    - Meaning: Destination EUI-48 MAC address
    - Type: MAC
    - Optional: yes (optional if `dip` is multicast)
    - Example: `dmac=11:22:33:44:55:66`

- Name: `smac`
    - Meaning: Source EUI-48 MAC address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `smac=80:12:34:45:67:89`

- Name: `dip`
    - Meaning: Destination IPv4 address
    - Type: IPv4 address
    - Example: `dip=1.2.3.4`

- Name: `sip`
    - Meaning: Source IPv4 address
    - Type: IPv4 address
    - Optional: yes — Default: local interface IP (if available)
    - Example: `sip=192.168.0.1`

- Name: `dscp`
    - Meaning: Differentiated Services Code Point
    - Type: Integer
    - Range: 0..0x3f
    - Optional: yes — Default: 0
    - Example: `dscp=16`

- Name: `ecn`
    - Meaning: Explicit Congestion Notification
    - Type: Integer
    - Range: 0..3
    - Optional: yes — Default: 0
    - Example: `ecn=1`

- Name: `ttl`
    - Meaning: Time To Live
    - Type: Integer
    - Range: 0..255
    - Optional: yes — Default: 64
    - Example: `ttl=200`

- Name: `df`
    - Meaning: Don't Fragment flag (1 = set)
    - Type: Integer (bit)
    - Optional: yes — Default: 0
    - Example: `df=1`

- Name: `id`
    - Meaning: Identification field
    - Type: Integer
    - Range: 0..0xffff
    - Optional: yes — Default: 0 (or auto for fragmented packets)
    - Example: `id=12345`

- Name: `protocol`
    - Meaning: [IP protocol number](https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml) (transport protocol).
    - Type: Integer
    - Range: 0..255
    - Example: `protocol=254`

- Name: `hchksum`
    - Meaning: IPv4 header checksum
    - Type: Integer
    - Range: 0..0xffff
    - Optional: yes — Behavior: If omitted checksum is calculated automatically; setting a value allows crafting malformed packets.
    - Example: `hchksum=42`

- Name: `payload`
    - Meaning: Encapsulated payload (transport header + data) or raw bytestream
    - Type: Bytestream / embedded packet
    - Note: For syntax and encoding options see `doc/PACKET_REFERENCE.md` (Bytestream, String, Random, Embedded packet syntax).
    - Example: `payload=12345678`

## Standards
RFC791, RFC2474
## Examples

raw simple ipv4 packet
```
ipv4(dmac=11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678)
```

the same packet combined with vlan tag
```
ipv4(vid=42, dmac=11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678)
```

raw fully defined IPv4 packet
```
ipv4(smac=80:12:34:45:67:89, dmac=11:22:33:44:55:66, sip=192.168.0.1, dip=172.16.1.2, ttl=200, dscp=16, ecn=1, df=1, protocol=254, payload=12345678)
```

raw simple ipv4 packet with multicast destination (dmac is therefore set automatically)
```
ipv4(dip=224.2.3.4, protocol=254, payload=12345678)
```

same as above, but dmac is forced
```
ipv4(dmac=11:22:33:44:55:66, dip=224.2.3.4, protocol=254, payload=12345678)
```