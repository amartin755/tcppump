# Link Layer Discovery Protocol (LLDP)

## Protocol Specifier
```
lldp
```

## Syntax
The specification uses a **descriptive, quasi-formal notation** intended for documentation and validation.
It is not a grammar definition.
See [Notation and Conventions](doc/PACKET_REFERENCE.md)

```
lldp(
    dmac               : MAC        [optional]
    smac               : MAC        [optional]
    CHASSIS-ID-TLV     : GROUP      [optional]
    PORT-ID-TLV        : GROUP      [optional]
    TTL-TLV            : GROUP      [optional]
    PORT-DESC-TLV      : GROUP      [optional]
    SYS-NAME-TLV       : GROUP      [optional]
    SYS-DESC-TLV       : GROUP      [optional]
    SYS-CAPS-TLV       : GROUP      [optional]
    MGT-ADDR-TLV       : GROUP      [optional]
    PVID-TLV           : GROUP      [optional]
    PPVID-TLV          : GROUP      [0..n]
    VLAN-NAME-TLV      : GROUP      [0..n]
    PROTO-ID-TLV       : GROUP      [0..n]
    VID-USE-TLV        : GROUP      [optional]
    MGT-VID-TLV        : GROUP      [optional]
    LAG-TLV            : GROUP      [optional]
    CONG-NOTE-TLV      : GROUP      [optional]
    ETS-CFG-TLV        : GROUP      [optional]
    ETS-REC-TLV        : GROUP      [optional]
    PFC-CFG-TLV        : GROUP      [optional]
    APP-PRIO-TLV       : GROUP      [optional]
    EVB-TLV            : GROUP      [optional]
    CDCP-TLV           : GROUP      [optional]
    APP-VLAN-TLV       : GROUP      [optional]
    MAC-PHY-CFG-TLV    : GROUP      [optional]
    POE-TLV            : GROUP      [optional]
    MAX-MTU-TLV        : GROUP      [optional]
    EEE-TLV            : GROUP      [optional]
    EEE-FW-TLV         : GROUP      [optional]
    PN-DELAY-TLV       : GROUP      [optional]
    PN-PORT-STAT-TLV   : GROUP      [optional]
    PN-ALIAS-TLV       : GROUP      [optional]
    PN-MRP-PSTAT-TLV   : GROUP      [optional]
    PN-CHASSIS-MAC-TLV : GROUP      [optional]
    PN-PTCP-STAT-TLV   : GROUP      [optional]
    PN-MAU-EXT-TLV     : GROUP      [optional]
    PN-MRPIC-PSTAT-TLV : GROUP      [optional]
    RAW-TLV            : GROUP      [0..n]
    RAW-OUI-TLV        : GROUP      [0..n]
)
```
Note: [VLAN tag parameters](doc/ethernet.md) are allowed optionally.

## Parameter reference
- Name: `dmac`
    - Meaning: Destination EUI-48 MAC address
    - Type: MAC
    - Optional: yes — Default: 01:80:C2:00:00:0E
    - Example: `dmac=12:23:34:34:44:44`

- Name: `smac`
    - Meaning: Source EUI-48 MAC address
    - Type: MAC
    - Optional: yes — Default: local interface MAC (if available)
    - Example: `smac=80:12:34:45:67:89`

---
### Chassis ID TLV (`CHASSIS-ID-TLV`)
```
chassis-id      : MAC | IP4 | IP6 | BYTESTREAM
chassis-id-type : INT8                              [optional]
```

- Name: `chassis-id`
    - Meaning: Chassis ID (EUI-48 MAC, IPv4/6 address, bytestream)
    - Type: auto (MAC / IPv4 / IPv6 / Bytestream), if parameter `chassis-id-type` is not set. Otherwise Bytestream
    - Optional: yes (only if parameter `chassis-id-type` is not set) — Default: interface MAC address
    - Example: `chassis-id=aa:bb:cc:dd:ee:ff`
    - Example: `chassis-id=1.2.3.4`
    - Example: `chassis-id="my-id"`

- Name: `chassis-id-type`
    - Meaning: Chassis ID Subtype
    - Type: Integer
    - Range: 0..255
    - Optional: yes — Behavior: Automatically set if parameter `chassis-id` is provided
    - Example: `chassis-id-type=4`

Note: The `chassis-id-type` is always set automatically based on the type of `chassis-id` provided. If `chassis-id` is not specified, both `chassis-id` and `chassis-id-type` are set automatically to the interface MAC address and the corresponding type. If `chassis-id-type` is explicitly provided, then `chassis-id` must be a bytestream.

#### Examples
Custom Chassis ID (IPv4 address, chassis-id-type automatically set to 5)
```
lldp(chassis-id=1.2.3.4)
```
Explicit chassis-id-type (chassis-id treated as bytestream)
```
lldp(chassis-id-type=7, chassis-id="custom-chassis")
```
---

### Port ID TLV (`PORT-ID-TLV`)
```
port-id      : MAC | IP4 | IP6 | BYTESTREAM
port-id-type : INT8                              [optional]
```

- Name: `port-id`
    - Meaning: Port ID (EUI-48 MAC, IPv4/6 address, bytestream)
    - Type: MAC / IPv4 / IPv6 / BYTESTREAM
    - Optional: yes (if parameter `port-id-type` is not set) — Default: interface MAC address
    - Example: `port-id=aa:bb:cc:dd:ee:ff`
    - Example: `port-id=1.2.3.4`
    - Example: `port-id="my-port-id"`

- Name: `port-id-type`
    - Meaning: Port ID Subtype
    - Type: Integer
    - Range: 0..255
    - Optional: yes — Behavior: Automatically set if parameter `port-id` is provided
    - Example: `port-id-type=3`

Note: The `port-id-type` is always set automatically based on the type of `port-id` provided. If `port-id` is not specified, both `port-id` and `port-id-type` are set automatically to the interface MAC address and the corresponding type. If `port-id-type` is explicitly provided, then `port-id` must be a BYTESTREAM.

---

### Time To Live TLV (`TTL-TLV`)
```
ttl : INT16
```

- Name: `ttl`
    - Meaning: Time to live
    - Type: Integer
    - Range: 0..65535
    - Optional: yes — Default: 120
    - Example: `ttl=300`
---

### Port Description TLV (`PORT-DESC-TLV`)
```
port-desc : BYTESTREAM
```

- Name: `port-desc`
    - Meaning: Port description
    - Type: String / Bytestream
    - Optional: yes — Behavior: If not set, the TLV will be omitted
    - Example: `port-desc="Ethernet Port 1"`
---

### System Name TLV (`SYS-NAME-TLV`)
```
sys-name : BYTESTREAM
```

- Name: `sys-name`
    - Meaning: System name
    - Type: String / Bytestream
    - Optional: yes — Behavior: If not set, the TLV will be omitted
    - Example: `sys-name="Switch01"`
---

### System Description TLV (`SYS-DESC-TLV`)
```
sys-desc : BYTESTREAM
```

- Name: `sys-desc`
    - Meaning: System description
    - Type: String / Bytestream
    - Optional: yes — Behavior: If not set, the TLV will be omitted
    - Example: `sys-desc="Binford 6100"`
---

### System Capabilities TLV (`SYS-CAPS-TLV`)
```
cap-other       : BIT  [optional]
cap-repeater    : BIT  [optional]
cap-bridge      : BIT  [optional]
cap-wlan-ap     : BIT  [optional]
cap-router      : BIT  [optional]
cap-phone       : BIT  [optional]
cap-docsis      : BIT  [optional]
cap-station     : BIT  [optional]
cap-cvlan       : BIT  [optional]
cap-svlan       : BIT  [optional]
cap-tpmr        : BIT  [optional]
encap-other     : BIT  [optional]
encap-repeater  : BIT  [optional]
encap-bridge    : BIT  [optional]
encap-wlan-ap   : BIT  [optional]
encap-router    : BIT  [optional]
encap-phone     : BIT  [optional]
encap-docsis    : BIT  [optional]
encap-station   : BIT  [optional]
encap-cvlan     : BIT  [optional]
encap-svlan     : BIT  [optional]
encap-tpmr      : BIT  [optional]
```

The System Capabilities TLV describes the capabilities supported by the device and which of those are currently enabled. Each capability is represented by a bit parameter, and each has a corresponding "Enabled" parameter to indicate its active status.


Each parameter can be set to `1` (enabled) or `0` (disabled). Bits that are not specified are automatically set to `0` and do not need to be provided explicitly.


| Parameter        | Meaning                                           |
|------------------|---------------------------------------------------|
| `cap-other`      | System Capability 'Other'                         |
| `cap-repeater`   | System Capability 'Repeater'                      |
| `cap-bridge`     | System Capability 'Bridge'                        |
| `cap-wlan-ap`    | System Capability 'WLAN AP'                       |
| `cap-router`     | System Capability 'Router'                        |
| `cap-phone`      | System Capability 'Telephone'                     |
| `cap-docsis`     | System Capability 'DOCSIS cable device'           |
| `cap-station`    | System Capability 'Station only'                  |
| `cap-cvlan`      | System Capability 'C-VLAN component'              |
| `cap-svlan`      | System Capability 'S-VLAN component'              |
| `cap-tpmr`       | System Capability 'Two-port MAC Relay component'  |
| `encap-other`    | Enabled Capability 'Other'                        |
| `encap-repeater` | Enabled Capability 'Repeater'                     |
| `encap-bridge`   | Enabled Capability 'Bridge'                       |
| `encap-wlan-ap`  | Enabled Capability 'WLAN AP'                      |
| `encap-router`   | Enabled Capability 'Router'                       |
| `encap-phone`    | Enabled Capability 'Telephone'                    |
| `encap-docsis`   | Enabled Capability 'DOCSIS cable device'          |
| `encap-station`  | Enabled Capability 'Station only'                 |
| `encap-cvlan`    | Enabled Capability 'C-VLAN component'             |
| `encap-svlan`    | Enabled Capability 'S-VLAN component'             |
| `encap-tpmr`     | Enabled Capability 'Two-port MAC Relay component' |



Example

```plaintext
lldp(cap-other, cap-bridge, cap-router, encap-other, encap-bridge, encap-router)
```

This example describes a device with the capabilities "Other", "Bridge", and "Router", all of which are marked as enabled.
All unspecified bits are automatically set to `0`.

**Note:**
If no capabilities are set, the TLV will not be generated.
The parser implementation checks that at least one capability or enabled status is set before adding the TLV.

---

### Management Address TLV (`MGT-ADDR-TLV`)
```
mgt-addr       : IP4 | IP6 | MAC | BYTESTREAM
mgt-addr-type  : INT8                           [optional]
if-number      : INT32                          [optional]
if-number-type : INT8                           [optional]
mgt-oid        : BYTESTREAM                     [optional]

```
Note: The TLV is only created if `mgt-addr` is provided. The value of `mgt-addr-type` is always set automatically based on the type of `mgt-addr` provided. If `if-number-type` is explicitly provided, then `mgt-addr` must be a BYTESTREAM.

- Name: `mgt-addr`
    - Meaning: Management Address
    - Type: auto (MAC / IPv4 / IPv6), if parameter `mgt-addr-type` is not set. Otherwise Bytestream
    - Example: `mgt-addr=aa:bb:cc:dd:ee:ff`
    - Example: `mgt-addr=1.2.3.4`
    - Example: `mgt-addr=2001:db8::1`

- Name: `mgt-addr-type`
    - Meaning: Management Address Subtype (see [ianaAddressFamilyNumbers](https://www.iana.org/assignments/address-family-numbers/address-family-numbers.xhtml) of RFC 3232 )
    - Type: Integer (8-bit)
    - Range: 0..255
    - Optional: yes — Behavior: Automatically set if parameter `mgt-addr` is provided
    - Example: `mgt-addr-type=4`

- Name: `if-number`
    - Meaning: Interface Number
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff
    - Optional: yes — Default: 0

- Name: `if-number-type`
    - Meaning: Interface Numbering Subtype: 1 = unknown, 2 = ifIndex, 3 = system port number
    - Type: Integer (8-bit)
    - Range: 0..255
    - Optional: yes — Default: 1

- Name: `mgt-oui`
    - Meaning: Organizationally Unique Identifier 
    - Type: Bytestream
    - Range: 3 bytes
    - Optional: yes — Default: empty
    - Example: `mgt-oui=0080c2`

#### Examples

Minimal TLV with IPv4 address
```
lldp(mgt-addr=1.2.3.4)
```

Manually specified AppleTalk address 10.1.50
```
lldp(mgt-addr=000a0132, mgt-addr-type=12)
```

MAC address as management-address, interface-number 1001 as ifIndex and OUI set to 0x0080c2
```
lldp(mgt-addr=aa:bb:cc:dd:ee:ff, if-number=1001, if-number-type=2, mgt-oui=0080c2)
```
---

### Port VLAN ID TLV (`PVID-TLV`)
```
pvid : INT16
```
Note: The TLV is only created if `pvid` is provided. Multiple TLV may be specified by repeating `pvid`.

- Name: `pvid`
    - Meaning: Port VLAN identifier
    - Type: Integer (16-bit)
    - Range: 0..65535
---

### Port And Protocol VLAN TLV (`PPVID-TLV`)
```
ppvid     : INT16
PPVID-SUP : BIT     [optional]
PPVID-EN  : BIT     [optional]
```
- Name: `ppvid`
    - Meaning: Port and protocol VLAN identifier
    - Type: Integer (16-bit)
    - Range: 0..65535

- Name: `PPVID-SUP`
    - Meaning: Port and Protocol VLAN supported
    - Type: Integer (bit)
    - Range: 0..1
    - Optional: yes — Default: 0

- Name: `PPVID-EN`
    - Meaning: Port and Protocol VLAN enabled
    - Type: Integer (bit)
    - Range: 0..1
    - Optional: yes — Default: 0

Note: The TLV is only created if `ppvid` is provided. Multiple TLVs may be specified by repeating the parameter group. For any TLV the `ppvid` parameter is mandatory.

#### Examples

PPVID 42 supported, but disabled
```
lldp(ppvid=42, PPVID-SUP=1)
```

PPVID 42 supported and enabled
```
lldp(ppvid=42, PPVID-SUP=1, PPVID-EN=1)
```

PPVID 42 and 43 supported and enabled
```
lldp(ppvid=42, PPVID-SUP=1, PPVID-EN=1, ppvid=43, PPVID-SUP=1, PPVID-EN=1)
```
---

### VLAN Name TLV (`VLAN-NAME-TLV`)
```
vlan-name-id : INT16
vlan-name    : BYTESTREAM
```
Note: The TLV is only created if `vlan-name` and `vlan-name-id` are provided. Multiple TLVs may be specified by repeating the parameter group. For any TLV both paramters are mandatory.

- Name: `vlan-name-id`
    - Meaning: VLAN ID
    - Type: Integer (16-bit)
    - Range: 0..65535

- Name: `vlan-name`
    - Meaning: VLAN name
    - Type: Bytestream
    - Range: max. 32 bytes

#### Examples

```
lldp(vlan-name-id=42, vlan-name="the-ultimate-vlan)
```
---
### Protocol Identity TLV (`PROTO-ID-TLV`)
```
proto-id : BYTESTREAM
```
Note: The TLV is only created if `proto-id` is provided.

- Name: `proto-id`
    - Meaning: Protocol Identity
    - Type: Bytestream
    - Range: max. 255 bytes

#### Examples
Identity for spanning tree protocol
```
lldp(proto-id=0026424203000000)
```

Identity for IPv4 protocol
```
lldp(proto-id=0800)
```
---

### VID Usage Digest TLV (`VID-USE-TLV`)
```
vid-usage-digest : INT32
```
Note: The TLV is only created if `vid-usage-digest` is provided.

- Name: `vid-usage-digest`
    - Meaning: VID Usage Digest
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff
---

### Management VID TLV (`MGT-VID-TLV`)
```
mgt-vid : INT16
```
Note: The TLV is only created if `mgt-vid` is provided.

- Name: `mgt-vid`
    - Meaning: Management VLAN ID
    - Type: Integer (16-bit)
    - Range: 0..65535
    - Example: `mgt-vid=42`

---

### Link Aggregation TLV (`LAG-TLV`)
```
lag-cap       : BIT
lag-status    : BIT [optional]
lag-port-type : INT [optional]
lag-port-id   : INT [optional]
```
Note: The TLV is only created if `lag-cap` is provided.

- Name: `lag-cap`
    - Meaning: Link aggregation capability (0 = not capable, 1 = capable)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `lag-status`
    - Meaning: Link aggregation status (0 = not currently in aggregation, 1 = currently in aggregation)
    - Type: Integer (bit)
    - Range: 0..1
    - Optional: yes — Default: 0

- Name: `lag-port-type`
    - Meaning: Aggregation Port Type (0 = no port type, 1 = Aggregation Port, 2 = Aggregator, 3 = Aggregator with single port)
    - Type: Integer
    - Range: 0..3
    - Optional: yes — Default: 0

- Name: `lag-port-id`
    - Meaning: Aggregated Port ID
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff
    - Optional: yes — Default: 0

#### Examples
Link Aggregation not supported
```
lldp(lag-cap=0)
```

Link Aggregation supported but disabled
```
lldp(lag-cap=1)
```

Link Aggregation supported, enabled, from aggregation port, port ID 42
```
lldp(lag-cap=1, lag-status=1, lag-port-type=1, lag-port-id=42)
```
---

### Congestion Notification TLV (`CONG-NOTE-TLV`)
```
cong-cnpv  : INT8
cong-ready : INT8
```
Note: The TLV is only created if `cong-cnpv` and `cong-ready` are provided.

- Name: `cong-cnpv`
    - Meaning: Per-priority CNPV indicators (capability)
    - Type: Integer (8-bit)
    - Range: 0..255

- Name: `cong-ready`
    - Meaning: Per-priority Ready indicators
    - Type: Integer (8-bit)
    - Range: 0..255

#### Examples
CNPV capability enabled for prio 0-3, ready for prio 0-1
```
lldp(cong-cnpv=0x0f, cong-ready=3)
```
---

### ETS Configuration TLV (`ETS-CFG-TLV`)
```
ets-cfg-willing : BIT
ets-cfg-cbs     : BIT
ets-cfg-max-tc  : INT
ets-cfg-prio    : INT32
ets-cfg-bw      : INT64
ets-cfg-tsa     : INT64
```
Note: The TLV is only created if all parameters are provided.

- Name: `ets-cfg-willing`
    - Meaning: Willing bit, if set, station accepts configurations
    - Type: Integer (bit)
    - Range: 0..1

- Name: `ets-cfg-cbs`
    - Meaning: Credit-based Shaper bit, if set, station supports CBS
    - Type: Integer (bit)
    - Range: 0..1

- Name: `ets-cfg-max-tc`
    - Meaning: Maximum number of traffic classes supported (0 = 8 TCs)
    - Type: Integer
    - Range: 0..7

- Name: `ets-cfg-prio`
    - Meaning: Priority Assignment Table
    - Type: Integer (32 bit)
    - Range: 0..0xffffffff

- Name: `ets-cfg-bw`
    - Meaning: TC Bandwidth Table
    - Type: Integer (64 bit)
    - Range: 0..0xffffffffffffffff

- Name: `ets-cfg-tsa`
    - Meaning: TSA Assignment Table
    - Type: Integer (64 bit)
    - Range: 0..0xffffffffffffffff
---

### ETS Recommendation TLV (`ETS-REC-TLV`)
```
ets-rec-prio    : INT32
ets-rec-bw      : INT64
ets-rec-tsa     : INT64
```
Note: The TLV is only created if all parameters are provided.

- Name: `ets-rec-prio`
    - Meaning: Priority Assignment Table
    - Type: Integer (32 bit)
    - Range: 0..0xffffffff

- Name: `ets-rec-bw`
    - Meaning: TC Bandwidth Table
    - Type: Integer (64 bit)
    - Range: 0..0xffffffffffffffff

- Name: `ets-rec-tsa`
    - Meaning: TSA Assignment Table
    - Type: Integer (64 bit)
    - Range: 0..0xffffffffffffffff
---

### Priority-based Flow Control Configuration TLV (`PFC-CFG-TLV`)
```
pfc-willing : BIT
pfc-mbc     : BIT
pfc-cap     : INT
pfc-enable  : INT
```
Note: The TLV is only created if all parameters are provided.

- Name: `pfc-willing`
    - Meaning: Willing bit, if set, station accepts configurations
    - Type: Integer (bit)
    - Range: 0..1

- Name: `pfc-mbc`
    - Meaning: MACsec Bypass Capability
    - Type: Integer (bit)
    - Range: 0..1

- Name: `pfc-cap`
    - Meaning: PFC Capability
    - Type: Integer
    - Range: 0..15

- Name: `pfc-enable`
    - Meaning: PFC Enable bit vector
    - Type: Integer
    - Range: 0..255

#### Examples
Accept Configurations; MACsec bypass is not supported; Priority-based flow control configuration is enabled for all 8 traffic classes
```
lldp(pfc-willing=1, pfc-mbc=0, pfc-cap=8, pfc-enable=0xff)
```
---

### Application Priority TLV (`APP-PRIO-TLV`)
```
appl-prio       : INT   [1..n]
appl-prio-sel   : INT   [1..n]
appl-prio-proto : INT16 [1..n]
```
Note: The TLV is only created if all parameters are provided. Multiple tuples of `appl-prio`, `appl-prio-sel` and `appl-prio-proto` may be specified by repeating the parameter group. 

- Name: `appl-prio`
    - Meaning: Priority
    - Type: Integer
    - Range: 0..7

- Name: `appl-prio-sel`
    - Meaning: Meaning of the protocol ID (1 = Ethertype, 2 = TCP/SCTP port, 3 = UDP port, 4 = UDP/TCP/SCTP/DCCP port, 5 = DSCP)
    - Type: Integer
    - Range: 0..7

- Name: `appl-prio-proto`
    - Meaning: Protocol ID
    - Type: Integer (16-bit)
    - Range: 0..65535

#### Examples
Prio 2 for UDP port 67 (DHCP) and prio 7 for Ethertype 0x1234
```
lldp(appl-prio=2, appl-prio-sel=3, appl-prio-proto=67, appl-prio=7, appl-prio-sel=1, appl-prio-proto=0x1234)
```
---

### EVB TLV (`EVB-TLV`)
```
evb-bridge-status  : INT8
evb-station-status : INT8
evb-max-retries    : INT
evb-rte            : INT
evb-mode           : INT
evb-rol-rwd        : BIT
evb-rwd            : INT
evb-rol-rka        : BIT
evb-rka            : INT
```
Note: The TLV is only created if all parameters are provided.

- Name: `evb-bridge-status`
    - Meaning: EVB capabilities that are supported by the EVB bridge
    - Type: Integer (8-bit)
    - Range: 0..255

- Name: `evb-station-status`
    - Meaning: EVB capabilities that are supported by the EVB station
    - Type: Integer (8-bit)
    - Range: 0..255

- Name: `evb-max-retries`
    - Meaning: maxRetries value for the ECP state machine
    - Type: Integer
    - Range: 0..7

- Name: `evb-rte`
    - Meaning: Retransmission exponent
    - Type: Integer
    - Range: 0..31

- Name: `evb-mode`
    - Meaning: EVB mode
    - Type: Integer
    - Range: 0..3

- Name: `evb-rol-rwd`
    - Meaning: Remote or Local flag for RWD value (0 = local, 1 = remote)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `evb-rwd`
    - Meaning: RWD value transmitted by the EVB bridge
    - Type: Integer
    - Range: 0..31

- Name: `evb-rol-rka`
    - Meaning: Remote or Local flag for RKA value (0 = local, 1 = remote)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `evb-rka`
    - Meaning: RKA value transmitted by the EVB station
    - Type: Integer
    - Range: 0..31

---

### CDCP TLV (`CDCP-TLV`)
```
cdcp-role   : BIT
cdcp-scomp  : BIT
cdcp-ch-cap : INT
cdcp-scid   : INT   [0..n]
cdcp-svid   : INT   [0..n]
```
Note: The TLV is only created if `cdcp-role`, `cdcp-scomp` and `cdcp-ch-cap` are provided. The parameters `cdcp-scid` and `cdcp-svid` can be specified multiple times in pairs, where each pair represents an S-channel with its index and VID.

- Name: `cdcp-role`
    - Meaning: Role (0 = Bridge, 1 = Station)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `cdcp-scomp`
    - Meaning: Presence of S-VLAN component for S-Channel
    - Type: Integer (bit)
    - Range: 0..1

- Name: `cdcp-ch-cap`
    - Meaning: Channel capacity
    - Type: Integer
    - Range: 0..4096

- Name: `cdcp-scid`
    - Meaning: Index number of S-channel
    - Type: Integer
    - Range: 0..4096
    - Optional: yes

- Name: `cdcp-svid`
    - Meaning: VID assigned to the S-channel
    - Type: Integer
    - Range: 0..4096
    - Optional: yes
---

### Application VLAN TLV (`APP-VLAN-TLV`)
```
appl-vlan-vid   : INT
appl-vlan-sel   : INT
appl-vlan-proto : INT16
```
Note: The TLV is only created if all parameters are provided. Multiple tuples of `appl-vlan-vid`, `appl-vlan-sel` and `appl-vlan-proto` may be specified by repeating the parameter group. 

- Name: `appl-vlan-vid`
    - Meaning: VLAN ID
    - Type: Integer
    - Range: 0..4095

- Name: `appl-vlan-sel`
    - Meaning: Meaning of the protocol ID (1 = Ethertype, 2 = TCP/SCTP port, 3 = UDP port, 4 = UDP/TCP/SCTP/DCCP port, 5 = DSCP)
    - Type: Integer
    - Range: 0..7

- Name: `appl-vlan-proto`
    - Meaning: Protocol ID
    - Type: Integer (16-bit)
    - Range: 0..65535

#### Examples
VID 2 for UDP port 67 (DHCP) and VID 7 for Ethertype 0x1234
```
lldp(appl-vlan-vid=2, appl-vlan-sel=3, appl-vlan-proto=67, appl-vlan-vid=7, appl-vlan-sel=1, appl-prio-vlan=0x1234)
```
---

### MAC/PHY Configuration/Status TLV (`MAC-PHY-CFG-TLV`)
```
mautype      : INT16
autoneg-sup  : BIT      [optional]
autoneg-en   : BIT      [optional]
autoneg-caps : INT16
```
Note: The TLV is only created if `mautype` and `autoneg-caps` are provided.

- Name: `mautype`
    - Meaning: Operational MAU type
    - Type: Integer (16-bit)
    - Range: 0..0xffff

- Name: `autoneg-sup`
    - Meaning: Auto-negotiation support
    - Type: Integer (bit)
    - Range: 0..1
    - Optional: yes — Default: 0

- Name: `autoneg-en`
    - Meaning: Auto-negotiation status
    - Type: Integer (bit)
    - Range: 0..1
    - Optional: yes — Default: 0

- Name: `autoneg-caps`
    - Meaning: PMD auto-negotiation advertised capability
    - Type: Integer (16-bit)
    - Range: 0..0xffff

#### Examples
100BaseTXFD Link, auto-negotiation supported and enabled, 10BaseTHD, 10BaseTFD, 100BaseTXHD, 100BaseTXFD advertised
```
lldp(mautype=16, autoneg-sup=1, autoneg-en=1, autoneg-caps=0x6c00)
```
---

### Power Via MDI TLV (`POE-TLV`)
```
BASIC           : GROUP
DLL-CLASS-EXT   : GROUP [optional]
```
Group `BASIC`
```
poe-port-class  : BIT
poe-power-sup   : BIT
poe-power-state : BIT
poe-pair-ctrl   : BIT
poe-power-pair  : INT8
poe-power-class : INT8
```
Group `DLL-CLASS-EXT`
```
poe-power-type  : INT
poe-power-src   : INT
poe-pd-4pid     : BIT
poe-power-prio  : INT
poe-req-power   : FLOAT
poe-alloc-power : FLOAT
```

Note: The TLV is only created if all parameters of group `BASIC` are provided. The group `DLL-CLASS-EXT` is only created if all its members are provided.

- Name: `poe-port-class`
    - Meaning: Port class (0 = PD, 1 = PSE)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `poe-port-sup`
    - Meaning: PSE MDI power support (0 = supported, 1 = not supported)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `poe-port-state`
    - Meaning: PSE MDI power state (0 = disabled, 1 = not enabled)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `poe-port-ctrl`
    - Meaning: PSE pairs control ability (0 = pair selection can not be controlled, 1 = pair selection can be controlled)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `poe-power-pair`
    - Meaning: PSE power pair field (1 = signal, 2 = spare)
    - Type: Integer (32-bit)
    - Range: 0..255

- Name: `poe-power-class`
    - Meaning: Power class (1 = Class 0 PD, 2 = Class 1 PD, ... , 5 = Class 4 and above PD)
    - Type: Integer (32-bit)
    - Range: 0..255

- Name: `poe-power-type`
    - Meaning: DLL power type (0 = Type 2 PSE, 1 = Type 2 PD, 2 = Type 1 PSE, 3 = Type 1 PD)
    - Type: Integer
    - Range: 0..3

- Name: `poe-power-src`
    - Meaning: DLL power source (Power type = PSE: 0 = unknown, 1 = primary, 2 = backup | Power type = PD: 0 = unknown, 1 = PSE, 3 = PSE and local)
    - Type: Integer
    - Range: 0..3

- Name: `poe-pd-4pid`
    - Meaning: PD 4PID (1 = PD supports powering of both Modes simultaneously, 0 = PD does not support...)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `poe-power-prio`
    - Meaning: DLL power priority (0 = unknown, 1 = critical, 2 = high, 3 = low)
    - Type: Integer
    - Range: 0..3

- Name: `poe-req-power`
    - Meaning: PD requested power value
    - Type: Float
    - Range: 0..6553.5

- Name: `poe-alloc-power`
    - Meaning: PSE allocated power value
    - Type: Float
    - Range: 0..6553.5

#### Examples
TODO

---

### Maximum Frame Size TLV (`MAX-MTU-TLV`)
```
max-frame-size : INT16
```
Note: The TLV is only created if `max-frame-size` is provided.

- Name: `max-frame-size`
    - Meaning: Maximum 802.3 frame size
    - Type: Integer (16-bit)
    - Range: 0..0xffff

#### Examples
MAC/PHY supports only basic frames
```
lldp(max-frame-size=1518)
```
MAC/PHY supports Q-tagged frames
```
lldp(max-frame-size=1522)
```
---

### Energy Efficient Ethernet TLV (`EEE-TLV`)
```
eee-tx-tw      : INT16
eee-rx-tw      : INT16
eee-fb-rx-tw   : INT16
eee-echo-tx-tw : INT16
eee-echo-rx-tw : INT16
```
Note: The TLV is only created if all parameters are provided.

- Name: `eee-tx-tw`
    - Meaning: EEE transmit Tw
    - Type: Integer (16-bit)
    - Range: 0..0xffff

- Name: `eee-rx-tw`
    - Meaning: EEE receive Tw
    - Type: Integer (16-bit)
    - Range: 0..0xffff

- Name: `eee-fb-rx-tw`
    - Meaning: EEE fallback receive Tw
    - Type: Integer (16-bit)
    - Range: 0..0xffff

- Name: `eee-echo-tx-tw`
    - Meaning: EEE echo transmit Tw
    - Type: Integer (16-bit)
    - Range: 0..0xffff

- Name: `eee-echo-rx-tw`
    - Meaning: eee-echo-rx-tw
    - Type: Integer (16-bit)
    - Range: 0..0xffff

---

### Energy Efficient Ethernet Fast Wake TLV (`EEE-FW-TLV`)
```
eee-tx-tw      : INT8
eee-rx-tw      : INT8
eee-fb-rx-tw   : INT8
eee-echo-tx-tw : INT8
```
Note: The TLV is only created if all parameters are provided.

- Name: `eee-tx-tw`
    - Meaning: Transmit fast wake
    - Type: Integer (8-bit)
    - Range: 0..255

- Name: `eee-rx-tw`
    - Meaning: Receive fast wake
    - Type: Integer (8-bit)
    - Range: 0..255

- Name: `eee-fb-rx-tw`
    - Meaning: Echo transmit fast wake
    - Type: Integer (8-bit)
    - Range: 0..255

- Name: `eee-echo-tx-tw`
    - Meaning: Echo receive fast wake
    - Type: Integer (8-bit)
    - Range: 0..255

---

### LLDP_PNIO_DELAY TLV (`PN-DELAY-TLV`)
```
pn-port-delay-rx     : INT32
pn-port-delay-rx-rem : INT32
pn-port-delay-tx     : INT32
pn-port-delay-tx-rem : INT32
pn-cable-delay       : INT32
```
Note: The TLV is only created if all parameters are provided.

- Name: `pn-port-delay-rx`
    - Meaning: PTCP_PortRxDelayLocal (nanoseconds)
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff

- Name: `pn-port-delay-rx-rem`
    - Meaning: PTCP_PortRxDelayRemote (nanoseconds)
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff

- Name: `pn-port-delay-tx`
    - Meaning: PTCP_PortTxDelayLocal (nanoseconds)
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff

- Name: `pn-port-delay-tx-rem`
    - Meaning: PTCP_PortTxDelayRemote (nanoseconds)
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff

- Name: `pn-cable-delay`
    - Meaning: Measured cable delay (nanoseconds)
    - Type: Integer (32-bit)
    - Range: 0..0xffffffff

---

### LLDP_PNIO_PORTSTATUS TLV (`PN-PORT-STAT-TLV`)
```
pn-rtc2-state        : INT
pn-rtc3-state        : INT
pn-rtc3-frag         : BIT
pn-rtc3-short-preamp : BIT
pn-rtc3-opt          : BIT
```
Note: The TLV is only created if all parameters are provided.

- Name: `pn-rtc2-state`
    - Meaning: RTClass2_PortStatus.State (0 = OFF, 1 = SYNC-DATA-LOADED, 2 = UP)
    - Type: Integer
    - Range: 0..3

- Name: `pn-rtc3-state`
    - Meaning: RTClass3_PortStatus.State (0 = OFF, 2 = UP, 4 = RUN)
    - Type: Integer
    - Range: 0..7

- Name: `pn-rtc3-frag`
    - Meaning: Fragmentation Mode (0 = disabled, 1 = enabled)
    - Type: Integer (bit)
    - Range: 0..1

- Name: `pn-rtc3-short-preamp`
    - Meaning: Short preample (0 = disabled (8 octets), 1 = enabled (1 octet))
    - Type: Integer (bit)
    - Range: 0..1

- Name: `pn-rtc3-opt`
    - Meaning: Optimized (0 = OFF, 1 = ON)
    - Type: Integer (bit)
    - Range: 0..1

---

### LLDP_PNIO_ALIAS TLV (`PN-ALIAS-TLV`)
```
pn-alias : BYTESTREAM
```
Note: The TLV is only created if `pn-alias` is provided.

- Name: `pn-alias`
    - Meaning: Alias name value
    - Type: Bytestream
    - Range: max. 255 bytes

---

### LLDP_PNIO_MRPPORTSTATUS TLV (`PN-MRP-PSTAT-TLV`)
```
pn-mrp-domain      : BYTESTREAM
pn-mrp-domain-uuid : BYTESTREAM
pn-mrp-mrrt-state  : INT        [optional]
```
Note: The TLV is only created if either `pn-mrp-domain` or `pn-mrp-domain-uuid` is provided. Both parameters exclude each other. If `pn-mrp-domain` is provided, `pn-mrp-domain-uuid` is calculated automatically. If `pn-mrp-domain` is an empty string, `pn-mrp-domain-uuid` will be set to the default uuid `FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF`. 

- Name: `pn-mrp-domain`
    - Meaning: MRP domain name
    - Type: Bytestream

- Name: `pn-mrp-domain-uuid`
    - Meaning: MRP domain uuid
    - Type: UUID as String
    - Example: `pn-mrp-domain-uuid="c3d687fe-789e-03a1-acdb-e5bfcbbc27b6"`

- Name: `pn-mrp-mrrt-state`
    - Meaning: MRRT port status (0 = OFF, 1 = CONFIGURED, 2 = UP)
    - Type: Integer
    - Range: 0..3
    - Optional: yes — Default: 0

#### Examples
Default domain
```
lldp(pn-mrp-domain="")
```
UUID calculated based on domain name
```
lldp(pn-mrp-domain="mrpdomain-1")
```
Explicitely set domain UUID
```
lldp(pn-mrp-domain-uuid="c3d687fe-789e-03a1-acdb-e5bfcbbc27b655")
```
---

### LLDP_PNIO_CHASSIS_MAC TLV (`PN-CHASSIS-MAC-TLV`)
```
pn-chassis-mac : MAC
```
Note: The TLV is only created if `pn-chassis-mac` is provided.

- Name: `pn-chassis-mac`
    - Meaning: Chassis MAC address
    - Type: MAC

#### Examples
```
lldp(pn-chassis-mac=00:01:02:03:04:05)
```

---

### LLDP_PNIO_PTCPSTATUS TLV (`PN-PTCP-STAT-TLV`)
```
pn-ptcp-master-mac  : MAC
pn-ptcp-domain-uuid : BYTESTREAM [optional]
pn-ptcp-irdata-uuid : BYTESTREAM [optional]
pn-ptcp-period-len  : INT        [optional]
pn-ptcp-red-orange  : INT        [optional]
pn-ptcp-orange      : INT        [optional]
pn-ptcp-green       : INT        [optional]
```
Note: The TLV is only created if `pn-ptcp-master-mac` is provided.

- Name: `pn-ptcp-master-mac`
    - Meaning: PTCP master source MAC address
    - Type: MAC

- Name: `pn-ptcp-domain-uuid`
    - Meaning: PTCP domain UUID
    - Type: UUID as String
    - Example: `pn-ptcp-domain-uuid="c3d687fe-789e-03a1-acdb-e5bfcbbc27b6"`
    - Optional: yes — Default: `"00000000-0000-0000-0000-000000000000"`

- Name: `pn-ptcp-irdata-uuid`
    - Meaning: IRDATA UUID
    - Type: UUID as String
    - Example: `pn-ptcp-irdata-uuid="c3d687fe-789e-03a1-acdb-e5bfcbbc27b6"`
    - Optional: yes — Default: `"00000000-0000-0000-0000-000000000000"`

- Name: `pn-ptcp-period-len`
    - Meaning: Length of period (nanoseconds)
    - Type: Integer
    - Range: 0..0x7fffffff
    - Optional: yes — Default: 0

- Name: `pn-ptcp-red-orange`
    - Meaning: Frame offset of red/orange period (nanoseconds)
    - Type: Integer
    - Range: 0..0x7fffffff
    - Optional: yes — Default: 0

- Name: `pn-ptcp-orange`
    - Meaning: Frame offset of orange period (nanoseconds)
    - Type: Integer
    - Range: 0..0x7fffffff
    - Optional: yes — Default: 0

- Name: `pn-ptcp-green`
    - Meaning: Frame offset of green period (nanoseconds)
    - Type: Integer
    - Range: 0..0x7fffffff
    - Optional: yes — Default: 0

---

### LLDP_PNIO_MAUTypeExtension TLV (`PN-MAU-EXT-TLV`)
```
pn-mautype-ext : INT16
```
Note: The TLV is only created if `pn-mautype-ext` is provided.

- Name: `pn-mautype-ext`
    - Meaning: MAUTYPE extension
    - Type: Integer (-bit)
    - Range: 0..0xffff

#### Examples
POF (Polymeric Optical Fiber)
```
lldp(pn-mautype-ext=0x100)
```

APL (Advanced Physical Layer)
```
lldp(pn-mautype-ext=0x200)
```

---

### LLDP_PNIO_MRPICPORT_STATUS TLV (`PN-MRPIC-PSTAT-TLV`)
```
pn-mrp-ic-domain-id : INT16
pn-mrp-ic-role      : INT16 [optional]
pn-mrp-ic-mic-pos   : INT16 [optional]
```
Note: The TLV is only created if `pn-mrp-ic-domain-id` is provided.

- Name: `pn-mrp-ic-domain-id`
    - Meaning: MRP interconnection domain identifier
    - Type: Integer (16-bit)
    - Range: 0..0xffff

- Name: `pn-mrp-ic-role`
    - Meaning: MRP interconnection role (0 = none, 1 = client, 2 = manager)
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Optional: yes — Default: 0

- Name: `pn-mrp-ic-mic-pos`
    - Meaning: MRP interconnection mic position (0 = Primary, 1 = Secondary)
    - Type: Integer (16-bit)
    - Range: 0..0xffff
    - Optional: yes — Default: 0

### Raw LLDP TLV (`RAW-TLV`)
```
type  : INT
value : BYTESTREAM
```
Note: The TLV is only created if `type` and `value` are provided and `type` must be the first paramater. Multiple TLVs may be specified by repeating the parameter group. For any TLV both paramters are mandatory.

- Name: `type`
    - Meaning: Raw TLV Type Number
    - Type: Integer
    - Range: 0..127

- Name: `value`
    - Meaning: Raw TLV Value as bytestream
    - Type: Bytestream
    - Range: max. 511 bytes

---

### Raw Organizationally Specific TLV (`RAW-OUI-TLV`)
```
oui       : BYTESTREAM
oui-type  : INT8
oui-value : BYTESTREAM
```
Note: The TLV is only created if all parameters are provided and `oui` must be the first paramater. Multiple TLVs may be specified by repeating the parameter group.

- Name: `oui`
    - Meaning: Organizationally Unique Identifier 
    - Type: Bytestream
    - Range: 3 bytes
    - Optional: yes — Default: empty
    - Example: `oui=0080c2`

- Name: `oui-type`
    - Meaning: Organizationally Specific TLV Subtype Number
    - Type: Integer (8-bit)
    - Range: 0..255

- Name: `oui-value`
    - Meaning: Organizationally Specific TLV Value as bytestream
    - Type: Bytestream
    - Range: max. 507 bytes

---

## Standards
IEEE 802.1AB-2016, IEEE802.1Q-2022, IEEE802.3-2023, IEC61158-6-10

Supported TLVs
| TLV                          | Standard            | Notes           |
|------------------------------|---------------------|-----------------|
|                              | IEEE 802.1AB-2016   |                 |
| Chassis ID                   | *Clause 8.5.2*      |                 |
| Port ID                      | *Clause 8.5.3*      |                 |
| Time To Live                 | *Clause 8.5.4*      |                 |
| Port Description             | *Clause 8.5.5*      |                 |
| System Name                  | *Clause 8.5.6*      |                 |
| System Description           | *Clause 8.5.7*      |                 |
| System Capabilities          | *Clause 8.5.8*      |                 |
| Management Address           | *Clause 8.5.9*      |                 |
| Organizationally specific    | *Clause 8.6*        |                 |
|                              | IEEE 802.1Q-2022    |                 |
| Port VLAN ID                 | *D.2.1*             |                 |
| Port And Protocol VLAN       | *D.2.2*             |                 |
| VLAN Name                    | *D.2.3*             |                 |
| Protocol Identity            | *D.2.4*             |                 |
| VID Usage Digest             | *D.2.5*             |                 |
| Management VID               | *D.2.6*             |                 |
| Congestion Notification      | *D.2.7*             |                 |
| ETS Configuration            | *D.2.8*             |                 |
| ETS Recommendation           | *D.2.9*             |                 |
| Priority-based Flow Control Configuration      | *D.2.10*            |
| Application Priority         | *D.2.11*            |                 |
| EVB                          | *D.2.12*            |                 |
| CDCP                         | *D.2.13*            |                 |
| Application VLAN             | *D.2.14*            |                 |
|                              | IEEE 802.1AX        |                 |
| Link Aggregation             | *F.2*               |                 |
|                              | IEEE802.3-2022      | Not supported: 'Additional Ethernet Capabilies' and 'Power via MDI Measurements' |
| MAC/PHY Configuration/Status | *Clause 79.3.1*     |
| Power Via MDI                | *Clause 79.3.2*     | Not supported: Type 3 and Type 4 extensions |
| Maximum Frame Size           | *Clause 79.3.4*     |
| EEE                          | *Clause 79.3.5*     |
| EEE Fast Wake                | *Clause 79.3.6*     |
|                              | IEC 61158-6-10      | Not supported: LLDP_PNIO_NMEDomainUUID, LLDP_PNIO_NMEManagementAddr, LLDP_PNIO_NMENameUUID, LLDP_PNIO_NMEParameterUUID, LLDP_PNIO_1ASWorkingClock, LLDP_PNIO_1ASGlobalTime  |
| LLDP_PNIO_DELAY              | *4.11.2.2*          |  | 
| LLDP_PNIO_PORTSTATUS         | *4.11.2.2*          |  | 
| LLDP_PNIO_ALIAS              | *4.11.2.2*          |  | 
| LLDP_PNIO_MRPPORTSTATUS      | *4.11.2.2*          |  | 
| LLDP_PNIO_CHASSIS_MAC        | *4.11.2.2*          |  | 
| LLDP_PNIO_PTCPSTATUS         | *4.11.2.2*          |  | 
| LLDP_PNIO_MAUTypeExtension   | *4.11.2.2*          |  | 
| LLDP_PNIO_MRPICPORT_STATUS   | *4.11.2.2*          |  | 

## Examples
Minimal stardard conforming packet with default values for Chassis ID, Port ID and Time To Live
```
lldp()
```
Custom Chassis ID explicitely set, port description and system capabilities
```
lldp(chassis-id=1.2.3.4, port-desc="Ethernet Port 1", sys-name="Switch01", cap-bridge, encap-bridge)
```
