# 0.4.0
_UNRELEASED_
- new: protocol: IPv6 support with limitations (no fragmentation). Can be used as transport for UDP, GRE and VXLAN.
- new: protocol: LLDP support. Supported TLVs: IEEE 802.1AB-2016, IEEE802.1Q-2022, IEEE802.3-2023, Profinet
- new: backend: ASCII backend. In addition to the output as a pcap file, a pure text output is now also possible. The file format can be specified using the -F option. Note: the -o option has been renamed to -w
- new: compiler: basic datatypes for raw protocol. Pakets can be set up via generic building blocks, like integer data types, IP adresses and others. NOTE: the parameter "payload" is no longer available. "stream" must be used instead.
- new: build: Increased default warning level to /W4 on MSVC
- new: command line: new parameter 'help [protocol]' prints available protocols and syntax.
- new: command line: -i parameter is no longer mandatory, if -w option is used
- new: build: Release builds with _FORTIFY_SOURCE=2
- new: build: cppcheck build option (WITH_CPPCHECK)
- new: test: Redesign of ctest environment with json based testcase definition
- new: compiler: Random numbers can range restrictions. E.g. '*[12-24]'
- new: packaging: native deb and rpm-package creation without CPack.
- Bugfix: backend: Use MTU of interface as default max. length for ethernet packets
- Bugfix: build: Build no longer depends on git

# 0.3.1
_2024-12-29 18:53:01 +0100_
- Many bug fixes
  - unaligned accesses
  - memory corruptions
  - undefined behaviour in release builds
  - memory leaks
- Build support for CLANG
- Build options for ASAN, UBSAN and valgrind
- Privileges are no longer needed, when packets are written to pcap files.

# 0.3.0
_2021-08-08 13:19:24 +0200_
- Bugfix: Fxed memory leak in case of packet compile errors
- Bugfix: In some cases it wasn't possible to execute tcppump on interfaces where no ip address was set. MAC and IP address of the interface are now only mandatory, if packet definition depends on them. Thanks to Sascha
- Configureable MTU (--mtu option)
- IP- and MAC addresses can now be defined as random values in packets (e.g. smac=*)
- Support for recursive packet definitions. Means, a packet can be defined as a payload of an other packet (via <> operator). This can be useful for encapsulation protocols like vxlan and GRE or for embedded payloads of ICMP packets.
- New protocols: vxlan, GRE

# 0.2.1
_2021-04-18 12:29:30 +0200_
- Bugfix: Wrong packet timing in pcap replay.
- Bugfix: Unknown packet parameters will no longer be silently ignored
- Speed of pcap replaying can be controlled via optional parameter --pcap option

# 0.2.0
_2021-03-31 11:51:48 +0200_
- new protocols
  - STP/RSTP
  - UDP
  - TCP
  - IGMP
  - ICMP
- IPv4 fragmentation support
- IPv4 multicast support
- Packet payload accepts strings like "Hello World" now
- Infinite loops via option -l0
- Random source- and destination MAC addresses (options --rand-smac and --rand-dmac)
- Changed default time granularity to milliseconds (-t option)
- Automatic ARP resolution of unknown destination IPv4 addresses (option --arp)
- Overwrite destination MAC address of all packets to MAC via new option --overwrite-dmac
- Responder mode (option --responder) which allows mirroring of received packets or receive triggered transmission of custom packets
- many bugfixes

# 0.1.0
_2020-09-11 20:17:42 +0200_

Initial release