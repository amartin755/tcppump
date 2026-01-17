# Ethernet II / IEEE 802.3 packet format

## Overview

This document describes the `eth(...)` packet specification used by *tcppump* to generate Ethernet frames.

Supported formats:

- Ethernet II (Ethertype)
- IEEE 802.3 without LLC
- IEEE 802.3 with LLC
- IEEE 802.3 with LLC + SNAP
- IEEE 802.1Q VLAN tagging (single and stacked / Q-in-Q)

## Protocol Specifier
```
eth
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
eth(
    dmac        : MAC
    smac        : MAC        [optional]

    VLAN        : GROUP      [0..n]
    LLC         : GROUP      [optional]
    SNAP        : GROUP      [optional]

    ethertype   : INT16      [optional]

    payload     : BYTESTREAM
)
```

## Parameter reference

- Name: `dmac`
    - Meaning: Destination EUI-48 MAC address
    - Type: MAC
    - Example: `dmac=11:22:33:44:55:66`

- Name: `smac`
    - Meaning: Source EUI-48 MAC address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `smac=aa:bb:cc:dd:ee:ff`

- Name: `payload`
    - Meaning: Frame payload / payload bytes carried by Ethernet
    - Type: Bytestream / embedded packet
    - Example: `payload=1234567890abcdef`
    - Example: `payload="Hello World"`

- Name: `ethertype`
    - Meaning: Ethertype for Ethernet II frames, or explicit 16-bit value in the EtherType/Length field
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Optional: yes — Behavior: If ommited or LLC header is defined, length will be calculated based on `payload`.
    - Example: `ethertype=0x0800`

### `VLAN` (VLAN tagging parameters)

```
vid     : INT
prio    : INT        [optional]
dei     : INT        [optional]
vtype   : INT        [optional]
```
Multiple VLAN tags may be specified by repeating the VLAN parameter group. For any VLAN tag the `vid` parameter is mandatory; if `vid` is omitted the tag is not created and any accompanying VLAN parameters are ignored.

- Name: `vid`
    - Meaning: VLAN identifier (TCI.VID) — mandatory when adding a VLAN tag
    - Type: Integer
    - Range: 0..4095
    - Example: `vid=100`

- Name: `prio`
    - Meaning: Priority Code Point (TCI.PCP)
    - Type: Integer
    - Range: 0..7
    - Optional: yes — Default: 0
    - Example: `prio=3`

- Name: `dei`
    - Meaning: Drop Eligible Indicator (TCI.DEI)
    - Type: Integer (bit)
    - Range: 0..1
    - Optional: yes — Default: 0
    - Example: `dei=1`

- Name: `vtype`
    - Meaning: VLAN type (1 = Customer VLAN (C-TAG), 2 = Provider VLAN (S-TAG))
    - Type: Integer / Enum
    - Range: 1..2
    - Optional: yes — Default: 1
    - Example: `vtype=2`

Implementation note: A VLAN tag is encoded as a 4-byte TCI plus an EtherType of `0x8100` (C-TAG) or `0x88a8`/provider-specific for S-TAG depending on `vtype`.

### `LLC` (IEEE 802.2 parameters)

```
dsap    : INT8
ssap    : INT8
control : INT8 | INT16  [optional]
```
Using `dsap` and `ssap` instructs the generator to emit an IEEE 802.2 LLC header instead of using Ethertype. Both `dsap` and `ssap` must be present for LLC to be encoded; `control` is optional.

- Name: `dsap`
    - Meaning: Destination Service Access Point (LLC) — required when using LLC
    - Type: Integer (8-bit)
    - Range: 0..0xff
    - Example: `dsap=0xAA`

- Name: `ssap`
    - Meaning: Source Service Access Point (LLC) — required when using LLC
    - Type: Integer (8-bit)
    - Range: 0..0xff
    - Example: `ssap=0x34`

- Name: `control`
    - Meaning: LLC control field. Encoding uses 8-bit or 16-bit representation depending on LLC rules (8-bit, if bits 0 and 1 are set).
    - Type: Integer (8- or 16-bit)
    - Range: 0..0xffff
    - Optional: yes — Default: 3 (UI frame)
    - Example: `control=0x11`

### SNAP parameters (LLC with SNAP extension)

```
oui      : INT24
protocol : INT16
```
SNAP is encoded by emitting the standard LLC header with DSAP/SSAP set to the SNAP values and inserting the SNAP header (3 byte OUI + 2 byte protocol id). When `oui`/`protocol` are provided the generator will produce a SNAP header; do not provide `dsap`/`ssap` in this case as those are filled automatically.

- Name: `oui`
    - Meaning: Organizationally Unique Identifier (SNAP) — required when using SNAP
    - Type: Integer (24-bit)
    - Range: 0..0xffffff
    - Example: `oui=0x0080c2`

- Name: `protocol`
    - Meaning: SNAP protocol type (equivalent to Ethertype) — required when using SNAP
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Example: `protocol=0x0800`

## Encoding rules and interactions

- If `ethertype` is set to a value >= 0x0600, the frame is treated as Ethernet II and `ethertype` is used as Ethertype.
- If `ethertype` is omitted and LLC/SNAP parameters are not provided, the implementation may treat the field as a length (IEEE 802.3). Length is calculated from `payload` and tags.
- If `dsap`/`ssap` are provided: encode LLC header and (optionally) SNAP header. Do not set `ethertype` in this mode; length will usually be calculated.
- VLAN tags: when multiple `vid` groups are specified they are emitted in the order they appear. For double-tagging, supply two `vid` parameter sets: e.g. `vid=100, vtype=2, vid=42, prio=3`.

## Standards
- IEEE 802.3
- IEEE 802.2
- IEEE 802.1Q 

## Examples

Ethernet II:
```
eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, ethertype=0x0800, payload=1234567890abcdef)
```

IEEE 802.3 without explicitly defined LLC:
```
eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, payload=1234567890abcdef)
```

IEEE 802.3 with LLC:
```
eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, dsap=0x12, ssap=0x34, control=0x03, payload=1122)
```

Single VLAN tagged Ethernet II:
```
eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=42, ethertype=0x0800, payload=1234567890abcdef)
```

Double VLAN (S-VLAN then C-VLAN):
```
eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=100, vtype=2, vid=42, prio=3, ethertype=0x0800, payload=1234567890abcdef)
```

SNAP (LLC+SNAP):
```
eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, oui=0x0080c2, protocol=0x0800, payload=1234567890abcdef)
```
