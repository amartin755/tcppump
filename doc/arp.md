# Address Resolution Protocol

## Protocol Specifiers
```
arp
arp-probe
arp-announce
```
## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
arp(
    dip         : IP4        [required]
    sip         : IP4        [optional]
    dmac        : MAC        [optional]
    smac        : MAC        [optional]
    op          : INT        [optional]
)
```
```
arp-probe(
    dip         : IP4        [required]
)
```
```
arp-announce(
    dip         : IP4        [required]
)
```
Note: [VLAN tag parameters](doc/ethernet.md) are allowed optionally.

## Parameter reference

- Name: `dip`
    - Meaning: Destination IPv4 address (protocol target)
    - Type: IPv4 address
    - Example: `dip=1.2.3.4`

- Name: `op`
    - Meaning: ARP opcode (1 = request, 2 = reply)
    - Type: Integer
    - Range: 0..0xffff
    - Optional: yes — Default: 1
    - Example: `op=1`

- Name: `smac`
    - Meaning: Source EUI-48 MAC address
    - Type: MAC (6 bytes)
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `smac=10:22:33:44:55:66`

- Name: `sip`
    - Meaning: Source IPv4 address
    - Type: IPv4 address
    - Optional: yes — Default: local interface IP (if available)
    - Example: `sip=192.168.0.166`

- Name: `dmac`
    - Meaning: Destination EUI-48 MAC address (target hardware address)
    - Type: MAC (6 bytes)
    - Optional: yes — Default: 00:00:00:00:00:00
    - Example: `dmac=01:02:03:04:05:06`

## Standards
RFC 826

## Examples

full crafted arp packet
```
arp(op=1, smac=10:22:33:44:55:66, sip=192.168.0.166, dmac=01:02:03:04:05:06, dip=1.2.3.4)
```

who has 11.22.33.44
```
arp(dip=11.22.33.44)
```

arp-probe as specified in RFC 5227 (sender IP set to 0.0.0.0). Shorthand for arp(op=1, sip=0.0.0.0, dmac=00:00:00:00:00:00, dip=1.2.3.4)
```
arp-probe(dip=1.2.3.4)
```

arp-announce (gratuitous ARP). Shorthand for arp(op=1, sip=1.2.3.4, dmac=00:00:00:00:00:00, dip=1.2.3.4)
```
arp-announce(dip=1.2.3.4)
```

ARP over a single VLAN tag (C-VLAN)
```
arp(dip=11.22.33.44, vid=42, prio=3)
```
