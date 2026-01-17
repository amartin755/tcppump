# TCP

## Protocol Specifier
```
tcp
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
tcp(
    dmac          : MAC        [optional]
    dip           : IP4
    smac          : MAC        [optional]
    sip           : IP4         [optional]
    sport         : INT16
    dport         : INT16
    seq           : INT32
    ack           : INT32
    win           : INT16      [optional]
    urgptr        : INT16      [optional]
    FIN           : BIT        [optional]
    SYN           : BIT        [optional]
    RESET         : BIT        [optional]
    PUSH          : BIT        [optional]
    ACK           : BIT        [optional]
    URGENT        : BIT        [optional]
    ECN           : BIT        [optional]
    CWR           : BIT        [optional]
    NONCE         : BIT        [optional]
    chksum        : INT16      [optional]
    payload       : BYTESTREAM [optional]
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

- Name: `sport`
    - Meaning: TCP source port
    - Type: Integer (port)
    - Range: 0..0xffff
    - Example: `sport=12345`

- Name: `dport`
    - Meaning: TCP destination port
    - Type: Integer (port)
    - Range: 0..0xffff
    - Example: `dport=80`

- Name: `seq`
    - Meaning: Sequence number
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff
    - Example: `seq=123456`

- Name: `ack`
    - Meaning: Acknowledgement number
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff
    - Example: `ack=654321`

- Name: `win`
    - Meaning: Window size
    - Type: Integer
    - Range: 0..65535
    - Optional: yes — Default: 1024
    - Example: `win=4096`

- Name: `urgptr`
    - Meaning: Urgent pointer
    - Type: Integer
    - Range: 0..65535
    - Optional: yes — Default: 0
    - Example: `urgptr=0`

- Name: `FIN`, `SYN`, `RESET`, `PUSH`, `ACK`, `URGENT`, `ECN`, `CWR`, `NONCE`
    - Meaning: Control flags (1 = set)
    - Type: Integer (bit)
    - Optional: yes — Default: 0
    - Example: `SYN=1`, `ACK=1`

- Name: `chksum`
    - Meaning: TCP checksum
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Optional: yes — Behavior: If omitted checksum is calculated automatically; setting a value allows crafting malformed packets.
    - Example: `chksum=0`

- Name: `payload`
    - Meaning: TCP payload (bytestream or embedded packet)
    - Type: Bytestream / embedded packet
    - Optional: yes
    - Note: See `doc/PACKET_REFERENCE.md` for payload syntax options.
    - Example: `payload=12345678`

## Standards
RFC9293

## Examples

SYN packet (three-way handshake start)
```
tcp(dip=192.0.2.10, sport=12345, dport=80, seq=0, ack=0, SYN=1)
```

Established connection with payload
```
tcp(dip=192.0.2.10, sport=12345, dport=80, seq=1000, ack=2000, ACK=1, payload="GET / HTTP/1.0\r\n\r\n")
```

TCP packet with random payload and explicit checksum
```
tcp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=54321, dport=80, seq=1, ack=0, SYN=1, chksum=0, payload=*)
```
