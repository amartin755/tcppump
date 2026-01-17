# Virtual Router Redundancy Protocol (VRRP)

## Protocol Specifiers
```
vrrp
vrrp3
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
vrrp | vrrp3(
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    vrid        : INT8
    vrip        : IP4        [1..366]
    vrprio      : INT8       [optional]
    aint        : INT8       [optional]
    type        : INT        [optional]
    chksum      : INT16      [optional]
)
```

Note: [VLAN tag parameters](doc/ethernet.md) may be specified optionally.

## Parameter reference

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

- Name: `vrid`
    - Meaning: Virtual Router Identifier
    - Type: Integer (8-bit)
    - Range: 1..255
    - Example: `vrid=42`

- Name: `vrip`
    - Meaning: Virtual Router IPv4 address (one or more)
    - Type: IPv4 address
    - Example: `vrip=1.2.3.4, vrip=1.2.3.5`

- Name: `vrprio`
    - Meaning: Virtual Router Priority
    - Type: Integer (8-bit)
    - Range: 0..255
    - Optional: yes — Default: 100
    - Example: `vrprio=120`

- Name: `aint`
    - Meaning: Advertisement interval (units depend on protocol version)
    - Type: Integer
    - Range: v2: 0..255 (seconds); v3: 0..4095 (centiseconds)
    - Optional: yes — Default: v2:1s; v3:100cs
    - Example: `aint=3`

- Name: `type`
    - Meaning: VRRP packet type
    - Type: Integer
    - Range: 0..15
    - Optional: yes — Default: 1 (note: v2 RFC restricts valid value to 1)
    - Example: `type=1`

- Name: `chksum`
    - Meaning: VRRP checksum
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Optional: yes — Behavior: If omitted checksum is calculated automatically
    - Example: `chksum=0x4321`

Note: Optionally all VLAN tag parameters and optional IPv4 parameters (see `doc/ethernet.md` and `doc/ipv4.md`) are allowed and can be combined with GRE fields.

## Standards
RFC3768, RFC5798

## Examples

VRRP V3 packet with default values; virtual router with id 42 and ip address 1.2.3.4. Source-mac and ip are taken from network interface
```
vrrp3(vrid=42, vrip=1.2.3.4)
```

The same as above with protocol version 2
```
vrrp(vrid=42, vrip=1.2.3.4)
```

fully defined VRRP2 packet with two virtual router ip addresses
```
vrrp(smac=80:12:34:45:67:89, sip=192.168.0.1, vrid=42, vrprio=120, vrip=1.2.3.4, vrip=1.2.3.5, aint=3)
```

malformed vrrp packet (undefined type)
```
vrrp(vrid=42, vrip=1.2.3.4, type=3)
```

malformed vrrp packet (wrong checksum)
```
vrrp(vrid=42, vrip=1.2.3.4, chksum=0x4321)
```
