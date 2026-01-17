# Raw IPv6 packet

Note: Fragmentation is not yet supported by the generator.

## Protocol Specifier
```
ipv6
```

## Syntax

The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
ipv6(
    dmac        : MAC        [optional]
    smac        : MAC        [optional]
    dip         : IP6
    sip         : IP6        [optional]
    dscp        : INT        [optional]
    ecn         : INT        [optional]
    ttl         : INT8       [optional]
    fl          : INT        [optional]
    protocol    : INT8
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
	- Meaning: Destination IPv6 address
	- Type: IPv6 address
	- Example: `dip=2001:db8::1`

- Name: `sip`
	- Meaning: Source IPv6 address
	- Type: IPv6 address
	- Optional: yes — Default: local interface IP (if available)
	- Example: `sip=2001:db8::2`

- Name: `protocol`
	- Meaning: IPv6 Next Header (transport/protocol identifier)
	- Type: Integer (8-bit)
	- Range: 0..255
	- Optional: no
	- Example: `protocol=17` (UDP)

- Name: `payload`
    - Meaning: Encapsulated payload (transport header + data) or raw bytestream
    - Type: Bytestream / embedded packet
    - Note: For syntax and encoding options see `doc/PACKET_REFERENCE.md` (Bytestream, String, Random, Embedded packet syntax).
    - Example: `payload=12345678`

- Name: `dscp`
	- Meaning: Differentiated Services Code Point (traffic class high bits)
	- Type: Integer
	- Range: 0..63
    - Optional: yes — Default: 0
	- Example: `dscp=10`

- Name: `ecn`
	- Meaning: Explicit Congestion Notification (traffic class low bits)
	- Type: Integer
	- Range: 0..3
    - Optional: yes — Default: 0
	- Example: `ecn=1`

- Name: `ttl`
	- Meaning: Hop Limit (TTL) for IPv6
	- Type: Integer (8-bit)
	- Range: 0..255
	- Optional: yes — Default: 64
	- Example: `ttl=128`

- Name: `fl`
	- Meaning: IPv6 Flow Label
	- Type: Integer (20-bit)
	- Range: 0..1048575
	- Optional: yes
	- Example: `fl=12345`

## Standards
RFC8200

## Examples

IPv6 with flow label, traffic class, custom source and random payload
```
ipv6(dip=ff02::1, protocol=58, smac=aa:bb:cc:dd:ee:ff, sip=2001:db8::5, dscp=10, ecn=0, fl=42, payload=*)
```
