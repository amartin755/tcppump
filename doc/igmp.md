# IGMP

## Protocol Specifier
```
igmp
igmp-query
igmp-report
igmp-leave
igmp3-query
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)

Note: Optionally all [VLAN tag parameters](doc/ethernet.md) and optional [IPv4](doc/ipv4.md) parameters are allowed.

### raw IGMPv1/v2
```
igmp(
    dmac        : MAC        [optional]
    dip         : IP4
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    type        : INT8       [optional]
    time        : INT8       [optional]
    chksum      : INT16      [optional]
    group       : IP4
    payload     : BYTESTREAM [optional]
)
```
### IGMPv2 General/Group-Specific Membership Query
```
igmp-query(
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    time        : INT8       [optional]
    group       : IP4        [optional]
)
```
### IGMPv2 Membership Report / IGMPv2 Leave Group
```
igmp-report | igmp-leave(
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    group       : IP4
)
```
### IGMPv3 General/Group-Specific/Group-and-Source-Specific Membership Query
```
igmp3-query(
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    time        : FLOAT      [optional]
    s           : BIT        [optional]
    qrv         : INT        [optional]
    qqic        : INT        [optional]
    group       : IP4        [optional]
    rsip        : IP4        [0..366]
)
```

## Parameter reference

- Name: `dmac`
  - Meaning: Destination EUI-48 MAC address
  - Type: MAC (6 bytes)
  - Optional: yes (optional if `dip` is multicast)
  - Example: `dmac=12:23:34:34:44:44`

- Name: `dip`
  - Meaning: Destination IPv4 address
  - Type: IPv4 address
  - Example: `dip=1.2.3.4`

- Name: `smac`
  - Meaning: Source EUI-48 MAC address
  - Type: MAC (6 bytes)
  - Optional: yes — Default: local interface MAC (if available)
  - Example: `smac=80:12:34:45:67:89`

- Name: `sip`
  - Meaning: Source IPv4 address
  - Type: IPv4 address
  - Optional: yes — Default: local interface IP (if available)
  - Example: `sip=192.168.0.1`

- Name: `type`
  - Meaning: IGMP Message Type
  - Type: Integer (8-bit)
  - Range: 0..255
  - Example: `type=0x11`

- Name: `time`
  - Meaning: Max Response Time
  - Type: Integer (8-bit) or Float (for IGMPv3)
  - Range: 0..255 (for IGMPv1/v2 in 1/10 seconds), 0..3174.4 (for IGMPv3 in seconds)
  - Optional: yes
  - Example: `time=100` (for IGMPv2, 10 seconds)

- Name: `chksum`
  - Meaning: IGMP checksum
  - Type: Integer (16-bit)
  - Range: 0..0xffff
  - Optional: yes — Behavior: If omitted checksum is calculated automatically
  - Note: NOT YET IMPLEMENTED
  - Example: `chksum=0x1a2b`

- Name: `group`
  - Meaning: Group IPv4 Address
  - Type: IPv4 address
  - Optional: yes (for queries, if omitted → General-Membership-Query)
  - Example: `group=224.0.0.1`

- Name: `payload`
  - Meaning: Additional payload (bytestream)
  - Type: Bytestream
  - Optional: yes
  - Note: NOT YET IMPLEMENTED
  - Example: `payload=12345678`

- Name: `s`
  - Meaning: S Flag (Suppress Router-Side Processing)
  - Type: Integer (8-bit)
  - Range: 0..1
  - Optional: yes — Default: 0
  - Example: `s=1`

- Name: `qrv`
  - Meaning: Querier's Robustness Variable
  - Type: Integer (8-bit)
  - Range: 0..7
  - Optional: yes — Default: 2
  - Note: According to RFC3376 zero is an invalid value
  - Example: `qrv=2`

- Name: `qqic`
  - Meaning: Querier's Query Interval Code in seconds
  - Type: Integer (16-bit)
  - Range: 0..31744
  - Optional: yes — Default: 125
  - Example: `qqic=125`

- Name: `rsip`
  - Meaning: Router Source IPv4 Address
  - Type: IPv4 address (multiple allowed, up to 366)
  - Optional: yes
  - Example: `rsip=192.0.2.1,192.0.2.2`

## Standards
RFC1112, RFC2236, RFC3376

## Examples

IGMPv2 Membership Report
```
igmp-report(group=224.0.0.1)
```

IGMPv2 Leave Group
```
igmp-leave(group=224.0.0.1)
```

IGMPv2 General Membership Query
```
igmp-query()
```

IGMPv2 Group-Specific Membership Query
```
igmp-query(group=224.0.0.1)
```

IGMPv3 General Membership Query
```
igmp3-query()
```

IGMPv3 Group-and-Source-Specific Membership Query
```
igmp3-query(group=224.0.0.1, rsip=192.0.2.1,192.0.2.2)
```

IGMPv3 Membership Report
```
igmp3-report(multicast=224.0.0.1, type=1, rsip=192.0.2.1)
```

Raw IGMPv2 packet
```
igmp(dip=224.0.0.1, type=0x11, group=224.0.0.1)
```
