# Full configurable raw Ethernet packet consisting of standard data types
For constructing arbitrary payloads (or embedding inside other protocols) use `raw`. The `eth` protocol is more suitable for full Ethernet frames; `raw` is intended for low-level byte composition.

## Protocol Specifier
```
raw
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
raw(
    byte        : INT8       [0..n]
    be16        : INT16      [0..n]
    le16        : INT16      [0..n]
    be32        : INT32      [0..n]
    le32        : INT32      [0..n]
    be64        : INT64      [0..n]
    le64        : INT64      [0..n]
    ip4         : IP4        [0..n]
    ip6         : IP6        [0..n]
    mac         : MAC        [0..n]
    stream      : BYTESTREAM [0..n]
)
```

All parameters are optional and can be used multiple times. The packet is compiled by adding parameter by parameter to the payload and therefore the order of the parameter is important.

## Parameter reference

- Name: `stream`
  - Meaning: Arbitrary bytestream appended to payload
  - Type: Bytestream
  - Optional: yes
  - Note: See `doc/PACKET_REFERENCE.md` for bytestream syntax (hex, quoted string, random `*` forms).
  - Example: `stream=1122334455` or `stream="Hello"`

- Name: `byte`
  - Meaning: Single byte value appended to payload
  - Type: Integer (8-bit)
  - Range: 0..255
  - Optional: yes
  - Example: `byte=0x55`

- Name: `be16`, `le16`
  - Meaning: 16-bit integer (big-/little-endian) appended to payload
  - Type: Integer (16-bit)
  - Range: 0..65535
  - Optional: yes
  - Example: `be16=0x1234`

- Name: `be32`, `le32`
  - Meaning: 32-bit integer (big-/little-endian) appended to payload
  - Type: Integer (32-bit)
  - Range: 0..4294967295
  - Optional: yes
  - Example: `be32=0x11223344`

- Name: `be64`, `le64`
  - Meaning: 64-bit integer (big-/little-endian) appended to payload
  - Type: Integer (64-bit)
  - Range: 0..18446744073709551615
  - Optional: yes
  - Example: `be64=0x0123456789abcdef`

- Name: `ip4`
  - Meaning: IPv4 address bytes appended to payload
  - Type: IPv4 address
  - Optional: yes
  - Example: `ip4=1.2.3.4`

- Name: `ip6`
  - Meaning: IPv6 address bytes appended to payload
  - Type: IPv6 address
  - Optional: yes
  - Example: `ip6=2001:db8::1`

- Name: `mac`
  - Meaning: EUI-48 MAC address bytes appended to payload
  - Type: MAC (6 bytes)
  - Optional: yes
  - Example: `mac=10:20:30:40:50:60`

## Examples

```
raw(stream=112233445566aabbccddeeff08001234567890abcdef)
raw(stream="Hello")
raw(stream=*64)
```
demo of all available data types
```
raw(byte=0x55, be16=0x1234, le16=0x1234, be32=0x11223344, le32=0x11223344,
    be64=0x0123456789abcdef, le64=0x0123456789abcdef, ip4=1.2.3.4,
    ip6=1002:3004:5006:7008:900A:B00C:D00E:F001, mac=10:20:30:40:50:60,
    stream="Hello World", byte=0xaa)
```

hand crafted ethernet packet. This is identical to `eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, ethertype=0x0800, payload=1234567890abcdef)`
```
raw(mac=11:22:33:44:55:66, mac=aa:bb:cc:dd:ee:ff, be16=0x0800, stream=1234567890abcdef)
```