# UDP

## Protocol Specifier
```
udp
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
udp | udp6(
    dmac          : MAC        [optional]
    dip           : IP4 | IP6
    smac          : MAC        [optional]
    sip           : IP4 | IP6  [optional]
    sport         : INT16
    dport         : INT16
    chksum        : INT16      [optional]
    payload       : BYTESTREAM [optional]
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
    - Type: IPv4 or IPv6 (`udp6`) address
    - Example: `dip=1.2.3.4`
    - Example: `dip=2001:db8::1`

- Name: `smac`
    - Meaning: Source EUI-48 MAC address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `smac=80:12:34:45:67:89`

- Name: `sip`
    - Meaning: Source IPv4 address
    - Type: IPv4 or IPv6 (`udp6`) address
    - Optional: yes — Default: local interface IP (if available)
    - Example: `sip=192.168.0.1`

- Name: `sport`
    - Meaning: UDP source port
    - Type: Integer (port)
    - Range: 0..0xffff
    - Example: `sport=1234`

- Name: `dport`
    - Meaning: UDP destination port
    - Type: Integer (port)
    - Range: 0..0xffff
    - Example: `dport=2345`

- Name: `chksum`
    - Meaning: UDP checksum
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Optional: yes — Behavior: If omitted checksum is calculated automatically; setting a value allows crafting malformed packets.
    - Example: `chksum=0`

- Name: `payload`
    - Meaning: UDP payload (bytestream or embedded packet)
    - Type: Bytestream / embedded packet
    - Optional: yes
    - Note: See `doc/PACKET_REFERENCE.md` for payload syntax options.
    - Example: `payload=12345678`

## Standards
RFC768

## Examples

UDP packet with source-mac and ip taken from network interface
```
udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=12345678)
```

UDP packet with explicit source-mac and ip
```
udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, smac=80:12:34:45:67:89, sip=192.168.0.1, sport=1234, dport=2345, payload=12345678)
```

UDP packet to multicast destination (dmac auto-set)
```
udp(dip=224.2.3.4, sport=1234, dport=2345, payload=12345678)
```

UDP packet without payload
```
udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345)
```

UDP packet with random payload
```
udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=*)
```

UDP packet embedded structured payload
```
udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345,
        payload=<raw(byte=0x55, be16=0x1234, le16=0x1234, be32=0x11223344)>)
```
