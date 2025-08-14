# 0.4.0
_UNRELEASED_

## Added
- compiler: IPv6 support with limitations (no fragmentation). Can be used as transport for UDP, GRE and VXLAN.
- compiler: LLDP support. Supported TLVs: IEEE 802.1AB-2016, IEEE802.1Q-2022, IEEE802.3-2023, Profinet
- backend: ASCII backend. In addition to the output as a pcap file, a pure text output is now also possible. The file format can be specified using the -F option. Note: the -o option has been renamed to -w
- compiler: Random numbers can range restrictions. E.g. '*[12-24]'
- compiler: basic datatypes for raw protocol. Pakets can be set up via generic building blocks, like integer data types, IP adresses and others.
- core: new parameter 'help [protocol]' prints available protocols and syntax.
- build: cppcheck build option (WITH_CPPCHECK)

## Changed
- packaging: native deb and rpm-package creation without CPack.
- core: -i parameter is no longer mandatory, if -w option is used
- test: Redesign of ctest environment with json based testcase definition
- build: Release builds with _FORTIFY_SOURCE=2
- build: Increased default warning level to /W4 on MSVC

## Fixed
- backend: Use MTU of interface as default max. length for ethernet packets
- build: Build no longer depends on git
- compiler: fixed oops if value is ommited on stream parameters

## Removed
- Responder mode and its related command line parameter --listener and --bpf-filter are no longer supported
- compiler: Renamed raw-protocol parameter "payload" to "stream".


# 0.3.1
_2024-12-29 18:53:01 +0100_

## Added
- build: support for CLANG
- build: build-options for ASAN, UBSAN (WITH_ASAN, WITH_UBSAN)
- test: memcheck support (valgrind)

## Changed
- core: Privileges are no longer needed, when packets are written to pcap files.

## Fixed
- unaligned accesses
- memory corruptions
- undefined behaviour in release builds
- memory leaks


# 0.3.0
_2021-08-08 13:19:24 +0200_

## Added
- core: Configureable MTU (--mtu option)
- core: IP- and MAC addresses can be defined as random values in packets (e.g. smac=*)
- compiler: Support for recursive packet definitions. Means, a packet can be defined as a payload of an other packet (via <> operator). This can be useful for encapsulation protocols like vxlan and GRE or for embedded payloads of ICMP packets.
- compiler:support for protocols vxlan, GRE

## Fixed
- Fixed memory leak in case of packet compile errors
- In some cases it wasn't possible to execute tcppump on interfaces where no ip address was set. MAC and IP address of the interface are now only mandatory, if packet definition depends on them. Thanks to Sascha


# 0.2.1
_2021-04-18 12:29:30 +0200_

## Added
- core: Speed of pcap replaying can be controlled via optional parameter --pcap option

## Fixed
- Wrong packet timing in pcap replay.
- Unknown packet parameters will no longer be silently ignored


# 0.2.0
_2021-03-31 11:51:48 +0200_

## Added
- compiler: new protocols STP/RSTP, UDP, TCP, IGMP, ICMPv4
- compiler: IPv4 fragmentation
- compiler: IPv4 multicast
- compiler: Packet payload accepts strings like "Hello World" now
- core: Infinite loops via option -l0
- core: Random source- and destination MAC addresses (options --rand-smac and --rand-dmac)
- core: Automatic ARP resolution of unknown destination IPv4 addresses (option --arp)
- core: Overwrite destination MAC address of all packets to MAC via new option --overwrite-dmac
- core: Responder mode (option --responder) which allows mirroring of received packets or receive triggered transmission of custom packets

## Changed
- core: Changed default time granularity to milliseconds (-t option)

## Fixed
- many bugfixes


# 0.1.0
_2020-09-11 20:17:42 +0200_

- Initial release