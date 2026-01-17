# Spanning Tree Protocol family (STP, RSTP)

## Protocol Specifiers
```
stp      # Configuration BPDU (STP)
rstp     # Configuration BPDU (RSTP)
stp-tcn  # Topology Change Notification BPDU (no parameters)
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)
```
stp | rstp(
    smac          : MAC        [optional]
    rbprio        : INT        [optional]
    rbidext       : INT        [optional]
    rbmac         : MAC        [optional]
    rpathcost     : INT        [optional]
    bprio         : INT        [optional]
    bidext        : INT        [optional]
    bmac          : MAC        [optional]
    pprio         : INT        [optional]
    pnum          : INT        [optional]
    msgage        : FLOAT      [optional]
    maxage        : FLOAT      [optional]
    hello         : FLOAT      [optional]
    delay         : FLOAT      [optional]
    topochange    : BIT        [optional]
    topochangeack : BIT        [optional]
    portrole      : INT        [optional]
    proposal      : BIT        [optional]
    learning      : BIT        [optional]
    forwarding    : BIT        [optional]
    agreement     : BIT        [optional]
)
```
```
stp-tcn()
```

`stp-tcn` has no parameters.

## Parameter reference

- Name: `smac`
    - Meaning: Source EUI-48 MAC address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `smac=aa:bb:cc:dd:ee:ff`

- Name: `rbprio`
    - Meaning: Root Bridge Priority
    - Type: Integer
    - Range: 0..15
    - Optional: yes — Default: 0
    - Example: `rbprio=0`

- Name: `rbidext`
    - Meaning: Root Bridge System ID Extension
    - Type: Integer
    - Range: 0..4095
    - Optional: yes — Default: 0
    - Example: `rbidext=0`

- Name: `rbmac`
    - Meaning: Root Bridge EUI-48 MAC Address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `rbmac=aa:bb:cc:dd:ee:ff`

- Name: `rpathcost`
    - Meaning: Root Path Cost
    - Type: Integer
    - Range: STP: 1..65535; RSTP: 1..4294967295
    - Optional: yes — Default: STP:4; RSTP:20000
    - Example: `rpathcost=4`

- Name: `bprio`
    - Meaning: Bridge Priority
    - Type: Integer
    - Range: 0..15
    - Optional: yes — Default: 8
    - Example: `bprio=8`

- Name: `bidext`
    - Meaning: Bridge System ID Extension
    - Type: Integer
    - Range: 0..4095
    - Optional: yes — Default: 0
    - Example: `bidext=0`

- Name: `bmac`
    - Meaning: Bridge EUI-48 MAC Address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `bmac=aa:bb:cc:dd:ee:ff`

- Name: `pprio`
    - Meaning: Port Priority
    - Type: Integer
    - Range: 0..15
    - Optional: yes — Default: 8
    - Example: `pprio=8`

- Name: `pnum`
    - Meaning: Port Number
    - Type: Integer
    - Range: 1..4095
    - Optional: yes — Default: 1
    - Example: `pnum=1`

- Name: `msgage`
    - Meaning: Message Age (seconds)
    - Type: Float
    - Range: 0.0..255.996
    - Optional: yes — Default: 0
    - Example: `msgage=0.0`

- Name: `maxage`
    - Meaning: Max Age (seconds)
    - Type: Float
    - Range: 0.0..255.996 (IEEE recommends 6..40)
    - Optional: yes — Default: 20
    - Example: `maxage=20.0`

- Name: `hello`
    - Meaning: Hello Time (seconds)
    - Type: Float
    - Range: 0.0..255.996
    - Optional: yes — Default: 2
    - Example: `hello=2.0`

- Name: `delay`
    - Meaning: Forward Delay (seconds)
    - Type: Float
    - Range: 0.0..255.996 (IEEE recommends 4..30)
    - Optional: yes — Default: 15
    - Example: `delay=15.0`

- Name: `topochange`, `topochangeack`
    - Meaning: Topology Change flags
    - Type: Integer (bit)
    - Optional: yes — Default: 0
    - Example: `topochange=1`

- Name: `portrole`
    - Meaning: (RSTP only) Port Role (1=Alternate/Backup, 2=Root, 3=Designated)
    - Type: Integer
    - Range: 1..3
    - Optional: yes — Default: 3
    - Example: `portrole=3`

- Name: `proposal`, `learning`, `forwarding`, `agreement`
    - Meaning: (RSTP only) control flags
    - Type: Integer (bit)
    - Optional: yes — Defaults: `proposal`=0, `learning`=1, `forwarding`=1, `agreement`=0
    - Example: `proposal=1`, `learning=0`

## Standards
IEEE802.1D-2004

## Examples

STP config BPDU with default parameters. MAC address of network interface is used as source address, bridge address and root bridge address
```
stp()
```

RSTP config BPDU with proposal and forwarding enabled (explicit fields)
```
rstp(smac=aa:bb:cc:dd:ee:ff, rbprio=0, rbmac=aa:aa:aa:aa:aa:aa, bprio=8, pnum=2, portrole=2, proposal=1, learning=1, forwarding=1)
```
