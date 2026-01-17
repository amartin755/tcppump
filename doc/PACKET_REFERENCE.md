# Packet Syntax Reference

tcppump's input consists of a list of Ethernet packets, defined using a human-readable C-like syntax. The list can either be passed directly as a command-line parameter or included in a script file (using the `-s` or `--script` parameter).

## Packet
### Abstract Format
Each packet is defined as follows (optional parameters are marked with brackets `[]`):

    [timestamp:] protocol([parameter_list])

* `timestamp`: A relative time value, specified as an integer followed by `:` (relative to the start of tcppump), e.g., `123456`. If the time starts with `+`, it is relative to the previous packet, e.g., `+100`.
* `protocol`: A protocol specifier (see *Protocol Definitions*).
* `parameter_list`: Protocol parameters enclosed in parentheses, provided as comma-separated parameter-value pairs: `parameter=value, parameter=value, ...`. Example: `(hugo=123, egon=456)`.

**Note:** All identifiers are case-sensitive.

### Parameters
Each protocol defines the names and types of its parameters (see *Protocol Definitions*). Depending on their type, parameter values can be:

* **Integer:** Decimal (`1234`), hexadecimal (`0x1234`), random number (`*`) or range restricted random number (`*[1-4]` `*[0x10-0x13]`).
* **Float:** Example: `1.2`
* **Bytestream:** A sequence of 8-bit values (bytes or octets), which can be defined as:

  - **ASCII Hex Values:** Each byte is represented as an ASCII-hex value (e.g., `01020304ABCD`).
  - **String:** A sequence of printable ASCII characters enclosed in double quotes (e.g., `"Hello World"`).
  - **Random:** `*` represents a random sequence of 32 bytes. A specific length can be defined by appending the desired byte length after the asterisk. For example, `*16` generates a random 16-byte sequence.
  - **Embedded Packet:** A fully defined packet can be embedded within another packet. The definition is enclosed in angle brackets `< >`, e.g., `<eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, payload=*)>`.

* **MAC Address:** A EUI-48 MAC address as six colon separated hexadecimal numbers (e.g., `12:23:34:45:56:67`). The entire MAC address or it components can also be random (`*`) or range restricted random (`*[80-8a]`)
* **IPv4 Address:** Example: `1.2.3.4`.  The whole address as well it's components can also be random (examples: `*`, `192.168.10.*` `192.168.*.*[100-2000]`)
* **IPv6 Address:** Example: `2001:db8::2`.

### Examples

    +1234:   protoMickey(color = 10, index = 0x16, msg = "Hello")

    1000000: protoMouse(payload = *32)

    donald(parZ = valueZ)

    doit()

    +1234:   protoMickey(color = 10, index = 0x16, msg = <protoMouse(payload = *32)>)

## Script files
Script files contain a list of packets as specified above. Each packet definition must be terminated with a semicolon (`;`). Comments start with `#`.

Example:

    # this is a comment
    +1234:   protoMickey(color = 10, index = 0x16, msg = "Hello");
    1000000: protoMouse(payload = *32);
    donald(parZ = valueZ);  # another comment
    doit();

## Protocol Definitions

Each protocol defines its parameters using a descriptive, quasi-formal
notation. This notation is intended for documentation and validation
purposes and is not a grammar definition.

Actual packet input always consists of a flat, comma-separated list of
`key=value` pairs as described above.

### Notation and Conventions

#### Parameters

    name : TYPE                 # mandatory parameter
    name : TYPE [optional]      # optional parameter
    name : TYPE [0..n]          # optional parameter, repeatable up to n times
    name : TYPE [1..n]          # mandatory parameter, optional repeatable up to n times


#### Types

    TYPE                    # single value type
    TYPE1 | TYPE2           # one of the listed types
    GROUP                   # logical parameter group (descriptive only)

Types such as `INT8`, `INT16`, `INT24`, `FLOAT`, `MAC`, `IP4`, `IP6`,
`BYTESTREAM` are defined globally.

Union types (e.g. `IP4 | IP6`) indicate that a parameter may accept
values of either type.

#### Groups

    name : GROUP [0..n]

A parameter of type `GROUP` represents a logical parameter group.
Groups are used to describe structure, repetition, and relationships
between parameters.

Group names are descriptive only and are **not written explicitly**
in packet input. Groups are inferred from the presence and combination
of their parameters.

Examples:

- A VLAN group is inferred when a `vid` parameter appears
- An LLC group is inferred when both `dsap` and `ssap` are present
- A SNAP group is inferred when both `oui` and `protocol` are present

#### General Rules

- Packet input consists of a flat list of `key=value` pairs
- Parameter order is generally insignificant unless explicitly stated
- Repeated parameters are processed in the order they appear
- Groups may impose additional constraints such as:
  - required parameters
  - mutual exclusion with other groups or parameters
  - implicit defaults

The following protocols are supported.

| Protocol Specifier | Protocol |
|--------------------|---------------------------------|
| `raw`              | [Raw Ethernet](raw.md) |
| `eth`              | [Ethernet II or IEEE802.3](ethernet.md) |
| `arp`</br>`arp-probe`</br>`arp-announce`              | [Address Resolution Protocol](arp.md) |
| `ipv4`             | [Raw IPv4](ipv4.md) |
| `ipv6`             | [Raw IPv6](ipv6.md) |
| `igmp`             | [IGMP with IPv4 transport](igmp.md) |
| `udp`              | [UDP with IPv4 transport](udp.md) |
| `udp6`             | [UDP with IPv6 transport](udp.md) |
| `tcp`              | [TCP with IPv4 transport](tcp.md) |
| `icmp`</br>`icmp-unreachable`</br>`icmp-src-quench`</br>`icmp-time-exceeded`</br>`icmp-redirect`</br>`icmp-echo`</br>`icmp-echo-reply`             | [ICMPv4](icmpv4.md) |
| `vrrp`             | [VRRPv2 with IPv4 transport](vrrp.md) |
| `vrrp3`            | [VRRPv3 with IPv4 transport](vrrp.md) |
| `stp`              | [Spanning Tree](stp.md) |
| `rstp`             | [Rapid Spanning Tree](stp.md) |
| `vxlan`            | [VXLAN with UDP/IPv4 transport](vxlan.md) |
| `vxlan6`           | [VXLAN with UDP/IPv6 transport](vxlan.md) |
| `gre`              | [GRE with IPv4 transport](gre.md) |
| `gre6`             | [GRE with IPv6 transport](gre.md) |
| `lldp`             | [LLDP](lldp.md) |


