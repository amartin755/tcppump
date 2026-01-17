# ICMPv4


## Protocol Specifier
```
icmp
icmp-unreachable
icmp-src-quench
icmp-time-exceeded
icmp-redirect
icmp-echo
icmp-echo-reply
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
icmp(
    dmac        : MAC        [optional]
    dip         : IP4
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    type        : INT8
    code        : INT8
    chksum      : INT16      [optional]
    payload     : BYTESTREAM [optional]
)
```
```
icmp-unreachable | icmp-src-quench | icmp-time-exceeded(
    dmac        : MAC        [optional]
    dip         : IP4
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    code        : INT8       [optional]
    payload     : BYTESTREAM [optional]
)
```
```
icmp-redirect(
    dmac        : MAC        [optional]
    dip         : IP4
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    gw          : IP4
    code        : INT8       [optional]
    payload     : BYTESTREAM [optional]
)
```
```
icmp-echo | icmp-echo-reply(
    dmac        : MAC        [optional]
    dip         : IP4
    smac        : MAC        [optional]
    sip         : IP4        [optional]
    id          : INT16      [optional]
    seq         : INT16      [optional]
    payload     : BYTESTREAM [optional]
)
```
Note: Optionally all [VLAN tag parameters](doc/ethernet.md) and optional [IPv4](doc/ipv4.md) parameters are allowed.

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

- Name: `type`
  - Meaning: ICMP type
  - Type: Integer (8-bit)
  - Range: 0..255
  - Example: `type=8`

- Name: `code`
  - Meaning: ICMP code (type-specific)
  - Type: Integer (8-bit)
  - Range: 0..255
  - Optional: yes — Default: 0 (where applicable)
  - Type-specific values (informational):
    - Destination Unreachable (type 3):
      - `0` = network unreachable
      - `1` = host unreachable
      - `2` = protocol unreachable
      - `3` = port unreachable
      - `4` = fragmentation needed and DF set
      - `5` = source route failed
      - `6` = destination network unknown
      - `7` = destination host unknown
      - `8` = source host isolated
      - `9` = network administratively prohibited
      - `10` = host administratively prohibited
      - `11` = network unreachable for TOS
      - `12` = host unreachable for TOS
      - `13` = communication administratively prohibited
      - `14` = host precedence violation
      - `15` = precedence cutoff in effect
    - Redirect (type 5):
      - `0` = redirect datagrams for the network
      - `1` = redirect datagrams for the host
      - `2` = redirect datagrams for the TOS and network
      - `3` = redirect datagrams for the TOS and host
    - Time Exceeded (type 11):
      - `0` = TTL exceeded in transit
      - `1` = fragment reassembly time exceeded
  - Example: `code=0`

- Name: `chksum`
  - Meaning: ICMP checksum
  - Type: Integer (16-bit)
  - Range: 0..0xffff
  - Optional: yes — Behavior: If omitted checksum is calculated automatically
  - Example: `chksum=0x1a2b`

- Name: `gw`
  - Meaning: Gateway IPv4 address (only for `icmp-redirect`)
  - Type: IPv4 address
  - Example: `gw=192.0.2.1`

- Name: `id`, `seq`
  - Meaning: Identifier and Sequence number (only for `icmp-echo`/`icmp-echo-reply`)
  - Type: Integer (16-bit)
  - Range: 0..65535
  - Optional: yes — Default: 0
  - Example: `id=1, seq=2`

- Name: `payload`
  - Meaning: ICMP payload (bytestream or embedded packet)
  - Type: Bytestream / embedded packet
  - Optional: yes
  - Note: See `doc/PACKET_REFERENCE.md` for payload syntax options.
  - Example: `payload=12345678`

## Standards
RFC792

## Examples

ICMP echo request
```
icmp-echo(dip=192.0.2.1, id=1, seq=1, payload="hello")
```

ICMP echo reply
```
icmp-echo-reply(dip=192.0.2.1, id=1, seq=1, payload="hello")
```

ICMP destination unreachable (port)
```
icmp-unreachable(dip=192.0.2.1, code=3, payload=<ipv4(dip=192.0.2.2, protocol=17, payload=...)>)
```
