# Virtual eXtensible Local Area Network (VXLAN)

## Protocol Specifier
```
vxlan
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
vxlan | vxlan6(
    dmac        : MAC        [optional]
    smac        : MAC        [optional]
    dip         : IP4 | IP6
    sip         : IP4 | IP6  [optional]
    sport       : INT16
    dport       : INT16      [optional]
    vni         : INT24      [optional]
    payload     : BYTESTREAM [optional]
)
```

Note: Optionally all [VLAN tag parameters](doc/ethernet.md) and optional [IPv4](doc/ipv4.md) or [IPv6](doc/ipv6.md) parameters are allowed.

## Parameter reference

- Name: `dmac`
    - Meaning: Destination EUI-48 MAC address
    - Type: MAC
    - Optional: yes (optional if `dip` is multicast)
    - Example: `dmac=12:23:34:34:44:44`

- Name: `dip`
    - Meaning: Destination IPv4 address
    - Type: IPv4 address
    - Example: `dip=1.2.3.4`

- Name: `smac`
    - Meaning: Source EUI-48 MAC address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `smac=80:12:34:45:67:89`

- Name: `sip`
    - Meaning: Source IPv4 address
    - Type: IPv4 address
    - Optional: yes — Default: local interface IP (if available)
    - Example: `sip=192.168.0.1`

- Name: `sport`
    - Meaning: UDP source port (outer transport)
    - Type: Integer (port)
    - Range: 0..0xffff
    - Example: `sport=1234`

- Name: `dport`
    - Meaning: UDP destination port (outer transport)
    - Type: Integer (port)
    - Range: 0..0xffff
    - Optional: yes — Default: 4789
    - Example: `dport=4789`

- Name: `vni`
    - Meaning: VXLAN Network Identifier
    - Type: Integer (24-bit)
    - Range: 0..0xffffff
    - Optional: yes — Default: 0
    - Example: `vni=42`

- Name: `payload`
    - Meaning: Encapsulated payload (Ethernet frame or bytestream)
    - Type: Bytestream / embedded packet
    - Optional: yes
    - Note: For payload formats see `doc/PACKET_REFERENCE.md`.
    - Example: `payload=1234567812345678`

## Standards
RFC7348

## Examples

VXLAN packet with source-mac and ip taken from network interface
```
vxlan(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, vni=42, payload=1234567812345678)
```

same VXLAN packet with embedded Ethernet II packet
```
vxlan(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, vni=42, payload=<eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, ethertype=0x8123, payload=1234567890abcdef);>)
```
