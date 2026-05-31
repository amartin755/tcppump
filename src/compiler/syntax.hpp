// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SYNTAX_HPP_
#define SYNTAX_HPP_

#include <array>

enum Type
{
    Invalid    = 0,
    Integer    = 1,       // generic integer with explicit range
    Int8       = 2,       // 8-bit integer, implicit range 0 ... 255
    Int16      = 4,       // 16-bit integer, implicit range 0 ... 65535
    Int32      = 8,       // 32-bit integer, implicit range 0 ... 4294967295
    Int64      = 16,      // 64-bit integer, implicit range 0 ... 18446744073709551615
    Bit        = 32,      // single bit, implicit range 0 ... 1
    Float      = 64,      // floating point number with explicit range
    Mac        = 128,     // EUI-48 MAC address
    IP4        = 256,     // IPv4 address
    IP6        = 512,     // IPv6 address
    UUID       = 1024,    // UUID value
    Bytestream = 2048,    // arbitrary bytestream with optional max length
    Nested     = 4096     // nested packet
};

struct ParameterSyntax
{
    const char* syntax;
    const char* descr;
    const int   type;
    const char* min = nullptr;
    const char* max = nullptr;
};

struct ProtocolSyntax
{
    const char* syntax;
    const char* descr;
    const ParameterSyntax* const* mandatory;
    const ParameterSyntax* const* optional;
};

using ParameterSyntaxArray = const ParameterSyntax* const[];

inline constexpr ParameterSyntaxArray empty_params = {nullptr};

inline constexpr ParameterSyntax PAR_RAW_BYTE = {
    "byte",
    "Raw byte value",
    Int8
};
inline constexpr ParameterSyntax PAR_RAW_BE16 = {
    "be16",
    "Big-endian 16-bit value",
    Int16
};
inline constexpr ParameterSyntax PAR_RAW_BE32 = {
    "be32",
    "Big-endian 32-bit value",
    Int32
};
inline constexpr ParameterSyntax PAR_RAW_BE64 = {
    "be64",
    "Big-endian 64-bit value",
    Int64
};
inline constexpr ParameterSyntax PAR_RAW_LE16 = {
    "le16",
    "Little-endian 16-bit value",
    Int16
};
inline constexpr ParameterSyntax PAR_RAW_LE32 = {
    "le32",
    "Little-endian 32-bit value",
    Int32
};
inline constexpr ParameterSyntax PAR_RAW_LE64 = {
    "le64",
    "Little-endian 64-bit value",
    Int64
};
inline constexpr ParameterSyntax PAR_RAW_IP4 = {
    "ip4",
    "IPv4 address",
    IP4
};
inline constexpr ParameterSyntax PAR_RAW_IP6 = {
    "ip6",
    "IPv6 address",
    IP6
};
inline constexpr ParameterSyntax PAR_RAW_MAC = {
    "mac",
    "EUI-48 Mac address",
    Mac
};
inline constexpr ParameterSyntax PAR_RAW_STREAM = {
    "stream",
    "Data stream",
    Bytestream
};
inline constexpr ParameterSyntaxArray PAR_RAW_OPT = {
    &PAR_RAW_BYTE,
    &PAR_RAW_BE16,
    &PAR_RAW_BE32,
    &PAR_RAW_BE64,
    &PAR_RAW_LE16,
    &PAR_RAW_LE32,
    &PAR_RAW_LE64,
    &PAR_RAW_IP4,
    &PAR_RAW_IP6,
    &PAR_RAW_MAC,
    &PAR_RAW_STREAM,
    nullptr
};

inline constexpr ProtocolSyntax PR_RAW = {
    "raw",
    "raw custom packet",
    empty_params,
    PAR_RAW_OPT
};


inline constexpr ParameterSyntax PAR_ETH_SMAC = {
    "smac",
    "Source EUI-48 Mac address",
    Mac
};
inline constexpr ParameterSyntax PAR_ETH_DMAC = {
    "dmac",
    "Destination EUI-48 Mac address",
    Mac
};
inline constexpr ParameterSyntax PAR_ETH_DSAP = {
    "dsap",
    "IEEE 802.2 DSAP field",
    Int8
};
inline constexpr ParameterSyntax PAR_ETH_SSAP = {
    "ssap",
    "IEEE 802.2 SSAP field",
    Int8
};
inline constexpr ParameterSyntax PAR_ETH_CONTROL = {
    "control",
    "Control field",
    Int16
};
inline constexpr ParameterSyntax PAR_ETH_OUI = {
    "oui",
    "Organizationally Unique Identifier",
    Integer,
    "0",
    "16777215"
};
inline constexpr ParameterSyntax PAR_ETH_PROTOCOL = {
    "protocol",
    "ProtocolSyntax identifier",
    Int16
};
inline constexpr ParameterSyntax PAR_ETH_PAYLOAD = {
    "payload",
    "Ethernet payload data",
    Bytestream
};
inline constexpr ParameterSyntax PAR_ETH_ETHERTYPE = {
    "ethertype",
    "EtherType field",
    Int16
};
inline constexpr ParameterSyntax PAR_ETH_VID = {
    "vid",
    "VLAN Identifier",
    Integer,
    "0",
    "4095"
};
inline constexpr ParameterSyntax PAR_ETH_VTYPE = {
    "vtype",
    "VLAN Type",
    Integer,
    "1",
    "2"
};
inline constexpr ParameterSyntax PAR_ETH_PRIO = {
    "prio",
    "VLAN Priority",
    Integer,
    "0",
    "7"
};
inline constexpr ParameterSyntax PAR_ETH_DEI = {
    "dei",
    "Drop Eligible Indicator",
    Bit
};
// shortcut for VLAN tag parameters
#define PAR_VLAN &PAR_ETH_VID, &PAR_ETH_VTYPE, &PAR_ETH_PRIO, &PAR_ETH_DEI

inline constexpr ParameterSyntaxArray PAR_ETH_MAN = {
    &PAR_ETH_DMAC,
    &PAR_ETH_PAYLOAD,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ETH_OPT = {
    &PAR_ETH_SMAC,
    &PAR_ETH_ETHERTYPE,
    PAR_VLAN,
    &PAR_ETH_DSAP,
    &PAR_ETH_SSAP,
    &PAR_ETH_CONTROL,
    &PAR_ETH_OUI,
    &PAR_ETH_PROTOCOL,
    nullptr
};

inline constexpr ProtocolSyntax PR_ETH = {
    "eth",
    "Ethernet II or IEEE802.3 packet",
    PAR_ETH_MAN,
    PAR_ETH_OPT
};


inline constexpr ParameterSyntax PAR_IP_DSCP = {
    "dscp",
    "Differentiated Services Code Point",
    Integer,
    "0",
    "63"
};
inline constexpr ParameterSyntax PAR_IP_ECN = {
    "ecn",
    "Explicit Congestion Notification",
    Integer,
    "0",
    "3"
};
inline constexpr ParameterSyntax PAR_IP_TTL = {
    "ttl",
    "Time To Live",
    Int8
};
inline constexpr ParameterSyntax PAR_IP4_DIP = {
    "dip",
    "Destination IPv4 address",
    IP4
};
inline constexpr ParameterSyntax PAR_IP4_SIP = {
    "sip",
    "Source IPv4 address",
    IP4
};
inline constexpr ParameterSyntax PAR_IP6_DIP = {
    "dip",
    "Destination IPv6 address",
    IP6
};
inline constexpr ParameterSyntax PAR_IP6_SIP = {
    "sip",
    "Source IPv6 address",
    IP6
};
inline constexpr ParameterSyntax PAR_IP_PROTOCOL = {
    "protocol",
    "Transport layer protocol",
    Int8
};
inline constexpr ParameterSyntax PAR_IP_PAYLOAD = {
    "payload",
    "IP packet payload",
    Bytestream
};
inline constexpr ParameterSyntax PAR_IP4_ID = {
    "id",
    "IPv4 packet identifier",
    Int16
};
inline constexpr ParameterSyntax PAR_IP4_DF = {
    "df",
    "IPv4 Don't Fragment flag",
    Bit
};
inline constexpr ParameterSyntax PAR_IP4_CHKSUM = {
    "hchksum",
    "IPv4 header checksum",
    Int16
};
inline constexpr ParameterSyntax PAR_IP6_FL = {
    "fl",
    "IPv6 Flow Label",
    Integer,
    "0",
    "1048575"
};
// shortcuts for IP header parameters
#define PAR_IP4_OPT &PAR_IP_DSCP, &PAR_IP_ECN, &PAR_IP_TTL, &PAR_IP4_DF, &PAR_IP4_SIP, &PAR_IP4_ID, &PAR_IP4_CHKSUM
#define PAR_IP4 &PAR_IP4_DIP
#define PAR_IP6_OPT &PAR_IP_DSCP, &PAR_IP_ECN, &PAR_IP_TTL, &PAR_IP6_SIP, &PAR_IP6_FL
#define PAR_IP6 &PAR_IP6_DIP

inline constexpr ParameterSyntaxArray PAR_IPV4_MANDATORY = {
    PAR_IP4,
    &PAR_IP_PROTOCOL,
    &PAR_IP_PAYLOAD,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_IPV4_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_ETH_DMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_IPV4 = {
    "ipv4",
    "Raw IPv4 packet",
    PAR_IPV4_MANDATORY,
    PAR_IPV4_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_IPV6_MANDATORY = {
    PAR_IP6,
    &PAR_IP_PROTOCOL,
    &PAR_IP_PAYLOAD,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_IPV6_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_ETH_DMAC,
    PAR_IP6_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_IPV6 = {
    "ipv6",
    "Raw IPv6 packet",
    PAR_IPV6_MANDATORY,
    PAR_IPV6_OPTIONAL
};


inline constexpr ParameterSyntax PAR_UDP_SPORT = {
    "sport",
    "Source UDP port",
    Int16
};
inline constexpr ParameterSyntax PAR_UDP_DPORT = {
    "dport",
    "Destination UDP port",
    Int16
};
inline constexpr ParameterSyntax PAR_UDP_PAYLOAD = {
    "payload",
    "UDP packet payload",
    Bytestream | Nested
};
inline constexpr ParameterSyntax PAR_UDP_CHKSUM = {
    "chksum",
    "UDP checksum",
    Int16
};
inline constexpr ParameterSyntaxArray PAR_UDP4_MANDATORY = {
    PAR_IP4,
    &PAR_UDP_SPORT,
    &PAR_UDP_DPORT,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_UDP4_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_ETH_DMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    &PAR_UDP_PAYLOAD,
    &PAR_UDP_CHKSUM,
    nullptr
};
inline constexpr ProtocolSyntax PR_UDP4 = {
    "udp",
    "IPv4 User Datagram ProtocolSyntax",
    PAR_UDP4_MANDATORY,
    PAR_UDP4_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_UDP6_MANDATORY = {
    PAR_IP6,
    &PAR_UDP_SPORT,
    &PAR_UDP_DPORT,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_UDP6_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_ETH_DMAC,
    PAR_IP6_OPT,
    PAR_VLAN,
    &PAR_UDP_PAYLOAD,
    &PAR_UDP_CHKSUM,
    nullptr
};
inline constexpr ProtocolSyntax PR_UDP6 = {
    "udp6",
    "IPv6 User Datagram ProtocolSyntax",
    PAR_UDP6_MANDATORY,
    PAR_UDP6_OPTIONAL
};


inline constexpr ParameterSyntax PAR_ARP_OP = {
    "op",
    "Opcode, 1 = request, 2 = reply",
    Int16
};
inline constexpr ParameterSyntaxArray PAR_ARP_MANDATORY = {
    &PAR_ARP_OP,
    &PAR_IP4_DIP,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ARP_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_ETH_DMAC,
    &PAR_IP4_SIP,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ARP = {
    "arp",
    "Raw ARP packet",
    PAR_ARP_MANDATORY,
    PAR_ARP_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_ARP_PROBE_MANDATORY = {
    &PAR_IP4_DIP,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ARP_PROBE_OPTIONAL = {
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ARP_PROBE = {
    "arp-probe",
    "ARP probe packet",
    PAR_ARP_PROBE_MANDATORY,
    PAR_ARP_PROBE_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_ARP_ANNOUNCE_MANDATORY = {
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ARP_ANNOUNCE_OPTIONAL = {
    &PAR_IP4_DIP,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ARP_ANNOUNCE = {
    "arp-announce",
    "ARP announce packet",
    PAR_ARP_ANNOUNCE_MANDATORY,
    PAR_ARP_ANNOUNCE_OPTIONAL
};


inline constexpr ParameterSyntax PAR_VRRP_VRIP = {
    "vrip",
    "Virtual Router IP address",
    IP4
};
inline constexpr ParameterSyntax PAR_VRRP_VRID = {
    "vrid",
    "Virtual Router ID",
    Integer,
    "1",
    "255"
};
inline constexpr ParameterSyntax PAR_VRRP_VRPRIO = {
    "vrprio",
    "Virtual Router Priority",
    Int8
};
inline constexpr ParameterSyntax PAR_VRRP_TYPE = {
    "type",
    "VRRP message type",
    Integer,
    "0",
    "15"
};
inline constexpr ParameterSyntax PAR_VRRP_AINT2 = {
    "aint",
    "Advertisement Interval",
    Int8
};
inline constexpr ParameterSyntax PAR_VRRP_AINT3 = {
    "aint",
    "Advertisement Interval",
    Integer,
    "0",
    "4095"
};
inline constexpr ParameterSyntax PAR_VRRP_CHKSUM = {
    "chksum",
    "VRRP checksum",
    Int16
};
inline constexpr ParameterSyntaxArray PAR_VRRP_MANDATORY = {
    &PAR_VRRP_VRIP,
    &PAR_VRRP_VRID,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_VRRP_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_IP4_SIP,
    &PAR_VRRP_VRPRIO,
    &PAR_VRRP_TYPE,
    &PAR_VRRP_AINT2,
    &PAR_VRRP_CHKSUM,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_VRRP = {
    "vrrp",
    "Virual Router Redundancy ProtocolSyntax V2",
    PAR_VRRP_MANDATORY,
    PAR_VRRP_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_VRRP3_MANDATORY = {
    &PAR_VRRP_VRIP,
    &PAR_VRRP_VRID,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_VRRP3_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_IP4_SIP,
    &PAR_VRRP_VRPRIO,
    &PAR_VRRP_TYPE,
    &PAR_VRRP_AINT3,
    &PAR_VRRP_CHKSUM,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_VRRP3 = {
    "vrrp3",
    "Virual Router Redundancy ProtocolSyntax V3",
    PAR_VRRP3_MANDATORY,
    PAR_VRRP3_OPTIONAL
};


inline constexpr ParameterSyntax PAR_STP_RBPRIO = {
    "rbprio",
    "Root Bridge Priority",
    Integer,
    "0",
    "15"
};
inline constexpr ParameterSyntax PAR_STP_RBIDEXT = {
    "rbidext",
    "Root Bridge ID Extension",
    Integer,
    "0",
    "4095"
};
inline constexpr ParameterSyntax PAR_STP_RBMAC = {
    "rbmac",
    "Root Bridge EUI-48 Mac address",
    Mac
};
inline constexpr ParameterSyntax PAR_STP_BPRIO = {
    "bprio",
    "Bridge Priority",
    Integer,
    "0",
    "15"
};
inline constexpr ParameterSyntax PAR_STP_BIDEXT = {
    "bidext",
    "Bridge ID Extension",
    Integer,
    "0",
    "4095"
};
inline constexpr ParameterSyntax PAR_STP_BMAC = {
    "bmac",
    "Bridge EUI-48 Mac address",
    Mac
};
inline constexpr ParameterSyntax PAR_STP_PPRIO = {
    "pprio",
    "Port Priority",
    Integer,
    "0",
    "15"
};
inline constexpr ParameterSyntax PAR_STP_PNUM = {
    "pnum",
    "Port Number",
    Integer,
    "1",
    "4095"
};
inline constexpr ParameterSyntax PAR_STP_MSGAGE = {
    "msgage",
    "Message Age",
    Float,
    "0.0",
    "255.996"
};
inline constexpr ParameterSyntax PAR_STP_MAXAGE = {
    "maxage",
    "Max Age",
    Float,
    "0.0",
    "255.996"
};
inline constexpr ParameterSyntax PAR_STP_HELLO = {
    "hello",
    "Hello Time",
    Float,
    "0.0",
    "255.996"
};
inline constexpr ParameterSyntax PAR_STP_DELAY = {
    "delay",
    "Forward Delay",
    Float,
    "0.0",
    "255.996"
};
inline constexpr ParameterSyntax PAR_STP_TOPOCHANGE = {
    "topochange",
    "Topology Change",
    Bit
};
inline constexpr ParameterSyntax PAR_STP_TOPOCHANGEACK = {
    "topochangeack",
    "Topology Change Acknowledgement",
    Bit
};
inline constexpr ParameterSyntax PAR_RSTP_RPATHCOST = {
    "rpathcost",
    "Root Path Cost",
    Integer,
    "1",
    "4294967295"
};
inline constexpr ParameterSyntax PAR_STP_RPATHCOST = {
    "rpathcost",
    "Root Path Cost",
    Integer,
    "1",
    "65535"
};
inline constexpr ParameterSyntax PAR_STP_PORTROLE = {
    "portrole",
    "Port Role",
    Integer,
    "1",
    "3"
};
inline constexpr ParameterSyntax PAR_STP_PROPOSAL = {
    "proposal",
    "Proposal",
    Bit
};
inline constexpr ParameterSyntax PAR_STP_LEARNING = {
    "learning",
    "Learning Mode",
    Bit
};
inline constexpr ParameterSyntax PAR_STP_FORWARDING = {
    "forwarding",
    "Forwarding Mode",
    Bit
};
inline constexpr ParameterSyntax PAR_STP_AGREEMENT = {
    "agreement",
    "Agreement",
    Bit
};
inline constexpr ParameterSyntaxArray PAR_STP_MANDATORY = {
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_STP_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_STP_RBPRIO,
    &PAR_STP_RBIDEXT,
    &PAR_STP_RBMAC,
    &PAR_STP_BPRIO,
    &PAR_STP_BIDEXT,
    &PAR_STP_BMAC,
    &PAR_STP_PPRIO,
    &PAR_STP_PNUM,
    &PAR_STP_MSGAGE,
    &PAR_STP_MAXAGE,
    &PAR_STP_HELLO,
    &PAR_STP_DELAY,
    &PAR_STP_TOPOCHANGE,
    &PAR_STP_TOPOCHANGEACK,
    &PAR_STP_RPATHCOST,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_STP = {
    "stp",
    "Spanning Tree ProtocolSyntax",
    PAR_STP_MANDATORY,
    PAR_STP_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_RSTP_MANDATORY = {
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_RSTP_OPTIONAL = {
    &PAR_ETH_SMAC,
    &PAR_STP_RBPRIO,
    &PAR_STP_RBIDEXT,
    &PAR_STP_RBMAC,
    &PAR_STP_BPRIO,
    &PAR_STP_BIDEXT,
    &PAR_STP_BMAC,
    &PAR_STP_PPRIO,
    &PAR_STP_PNUM,
    &PAR_STP_MSGAGE,
    &PAR_STP_MAXAGE,
    &PAR_STP_HELLO,
    &PAR_STP_DELAY,
    &PAR_STP_TOPOCHANGE,
    &PAR_STP_TOPOCHANGEACK,
    &PAR_RSTP_RPATHCOST,
    &PAR_STP_PORTROLE,
    &PAR_STP_PROPOSAL,
    &PAR_STP_LEARNING,
    &PAR_STP_FORWARDING,
    &PAR_STP_AGREEMENT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_RSTP = {
    "rstp",
    "Rapid Spanning Tree ProtocolSyntax",
    PAR_RSTP_MANDATORY,
    PAR_RSTP_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_STP_TCN_MANDATORY = {
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_STP_TCN_OPTIONAL = {
    nullptr
};
inline constexpr ProtocolSyntax PR_STP_TCN = {
    "stp-tcn",
    "STP Topology Change Notification",
    PAR_STP_TCN_MANDATORY,
    PAR_STP_TCN_OPTIONAL
};


inline constexpr ParameterSyntax PAR_IGMP_S = {
    "s",
    "Suppress Router-side Processing",
    Bit
};
inline constexpr ParameterSyntax PAR_IGMP_QRV = {
    "qrv",
    "Query Response Interval",
    Integer,
    "0",
    "7"
};
inline constexpr ParameterSyntax PAR_IGMP_QQIC = {
    "qqic",
    "Querier's Query Interval Count",
    Float,
    "0.0",
    "31744.0"
};
inline constexpr ParameterSyntax PAR_IGMP_TIME = {
    "time",
    "IGMP Time",
    Int8
};
inline constexpr ParameterSyntax PAR_IGMP_QUERY_TIME = {
    "time",
    "IGMP Time",
    Float,
    "0.0",
    "25.5"
};
inline constexpr ParameterSyntax PAR_IGMP3_QUERY_TIME = {
    "time",
    "IGMP Time",
    Float,
    "0.0",
    "3174.0"
};
inline constexpr ParameterSyntax PAR_IGMP_RSIP = {
    "rsip",
    "Router Source IP address",
    IP4
};
inline constexpr ParameterSyntax PAR_IGMP_GROUP = {
    "group",
    "Multicast group address",
    IP4
};
inline constexpr ParameterSyntax PAR_IGMP_TYPE = {
    "type",
    "IGMP message type",
    Int8
};
inline constexpr ParameterSyntaxArray PAR_IGMP_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    &PAR_IGMP_GROUP,
    &PAR_IGMP_TYPE,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_IGMP_OPTIONAL = {
    &PAR_IGMP_TIME,
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_IGMP = {
    "igmp",
    "Raw IGMP V1/V2 packet",
    PAR_IGMP_MANDATORY,
    PAR_IGMP_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_IGMP_QUERY_MANDATORY = {
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_IGMP_QUERY_OPTIONAL = {
    &PAR_IGMP_QUERY_TIME,
    &PAR_IGMP_GROUP,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_IGMP_QUERY = {
    "igmp-query",
    "IGMP V1/V2 Query",
    PAR_IGMP_QUERY_MANDATORY,
    PAR_IGMP_QUERY_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_IGMP3_QUERY_MANDATORY = {
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_IGMP3_QUERY_OPTIONAL = {
    &PAR_IGMP3_QUERY_TIME,
    &PAR_IGMP_GROUP,
    &PAR_IGMP_S,
    &PAR_IGMP_QRV,
    &PAR_IGMP_QQIC,
    &PAR_IGMP_RSIP,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_IGMP3_QUERY = {
    "igmp3-query",
    "IGMP V3 Query",
    PAR_IGMP3_QUERY_MANDATORY,
    PAR_IGMP3_QUERY_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_IGMP_REPORT_MANDATORY = {
    &PAR_IGMP_GROUP,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_IGMP_REPORT_OPTIONAL = {
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_IGMP_REPORT = {
    "igmp-report",
    "IGMP V1/V2 Report",
    PAR_IGMP_REPORT_MANDATORY,
    PAR_IGMP_REPORT_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_IGMP_LEAVE_MANDATORY = {
    &PAR_IGMP_GROUP,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_IGMP_LEAVE_OPTIONAL = {
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_IGMP_LEAVE = {
    "igmp-leave",
    "IGMP V1/V2 Leave",
    PAR_IGMP_LEAVE_MANDATORY,
    PAR_IGMP_LEAVE_OPTIONAL
};


inline constexpr ParameterSyntax PAR_ICMP4_TYPE = {
    "type",
    "ICMPv4 message type",
    Int8
};
inline constexpr ParameterSyntax PAR_ICMP4_CODE = {
    "code",
    "ICMPv4 message code",
    Int8
};
inline constexpr ParameterSyntax PAR_ICMP4_PAYLOAD = {
    "payload",
    "ICMPv4 message payload",
    Bytestream | Nested
};
inline constexpr ParameterSyntax PAR_ICMP4_CHKSUM = {
    "chksum",
    "ICMPv4 checksum",
    Int16
};
inline constexpr ParameterSyntax PAR_ICMP4_GW = {
    "gw",
    "Gateway address",
    IP4
};
inline constexpr ParameterSyntax PAR_ICMP4_ID = {
    "id",
    "ICMPv4 identifier",
    Int16
};
inline constexpr ParameterSyntax PAR_ICMP4_SEQ = {
    "seq",
    "ICMPv4 sequence number",
    Int16
};
inline constexpr ParameterSyntaxArray PAR_ICMP4_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    &PAR_ICMP4_TYPE,
    &PAR_ICMP4_CODE,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ICMP4_OPTIONAL = {
    &PAR_ICMP4_PAYLOAD,
    &PAR_ICMP4_CHKSUM,
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ICMP4 = {
    "icmp",
    "Raw ICMPv4 packet",
    PAR_ICMP4_MANDATORY,
    PAR_ICMP4_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_ICMP4_UNREACH_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ICMP4_UNREACH_OPTIONAL = {
    &PAR_ICMP4_CODE,
    &PAR_ICMP4_PAYLOAD,
    &PAR_ICMP4_CHKSUM,
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ICMP4_UNREACH = {
    "icmp-unreachable",
    "ICMPv4 Unreachable",
    PAR_ICMP4_UNREACH_MANDATORY,
    PAR_ICMP4_UNREACH_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_ICMP4_SRCQ_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ICMP4_SRCQ_OPTIONAL = {
    &PAR_ICMP4_CODE,
    &PAR_ICMP4_PAYLOAD,
    &PAR_ICMP4_CHKSUM,
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ICMP4_SRCQ = {
    "icmp-src-quench",
    "ICMPv4 Source Quench",
    PAR_ICMP4_SRCQ_MANDATORY,
    PAR_ICMP4_SRCQ_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_ICMP4_TIMEX_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ICMP4_TIMEX_OPTIONAL = {
    &PAR_ICMP4_CODE,
    &PAR_ICMP4_PAYLOAD,
    &PAR_ICMP4_CHKSUM,
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ICMP4_TIMEX = {
    "icmp-time-exceeded",
    "ICMPv4 Time Exceeded",
    PAR_ICMP4_TIMEX_MANDATORY,
    PAR_ICMP4_TIMEX_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_ICMP4_REDIR_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    &PAR_ICMP4_GW,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ICMP4_REDIR_OPTIONAL = {
    &PAR_ICMP4_CODE,
    &PAR_ICMP4_PAYLOAD,
    &PAR_ICMP4_CHKSUM,
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ICMP4_REDIR = {
    "icmp-redirect",
    "ICMPv4 Redirect",
    PAR_ICMP4_REDIR_MANDATORY,
    PAR_ICMP4_REDIR_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_ICMP4_ECHO_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ICMP4_ECHO_OPTIONAL = {
    &PAR_ICMP4_ID,
    &PAR_ICMP4_SEQ,
    &PAR_ICMP4_PAYLOAD,
    &PAR_ICMP4_CHKSUM,
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ICMP4_ECHO = {
    "icmp-echo",
    "ICMPv4 Echo Request (Ping)",
    PAR_ICMP4_ECHO_MANDATORY,
    PAR_ICMP4_ECHO_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_ICMP4_ECHOR_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_ICMP4_ECHOR_OPTIONAL = {
    &PAR_ICMP4_ID,
    &PAR_ICMP4_SEQ,
    &PAR_ICMP4_PAYLOAD,
    &PAR_ICMP4_CHKSUM,
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_ICMP4_ECHOR = {
    "icmp-echo-reply",
    "ICMPv4 Echo Reply",
    PAR_ICMP4_ECHOR_MANDATORY,
    PAR_ICMP4_ECHOR_OPTIONAL
};


inline constexpr ParameterSyntax PAR_TCP_SPORT = {
    "sport",
    "Source TCP port",
    Int16
};
inline constexpr ParameterSyntax PAR_TCP_DPORT = {
    "dport",
    "Destination TCP port",
    Int16
};
inline constexpr ParameterSyntax PAR_TCP_SEQ = {
    "seq",
    "TCP sequence number",
    Int32
};
inline constexpr ParameterSyntax PAR_TCP_ACK = {
    "ack",
    "TCP acknowledgment number",
    Int32
};
inline constexpr ParameterSyntax PAR_TCP_WIN = {
    "win",
    "TCP window size",
    Int16
};
inline constexpr ParameterSyntax PAR_TCP_URGPTR = {
    "urgptr",
    "TCP urgent pointer",
    Int16
};
inline constexpr ParameterSyntax PAR_TCP_FIN = {
    "FIN",
    "TCP FIN flag",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_SYN = {
    "SYN",
    "TCP SYN flag",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_RESET = {
    "RESET",
    "TCP RESET flag",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_PUSH = {
    "PUSH",
    "TCP PUSH flag",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_ACKFLAG = {
    "ACK",
    "TCP ACK flag",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_URGENT = {
    "URGENT",
    "TCP URGENT flag",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_ECN = {
    "ECN",
    "TCP ECN flag",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_CWR = {
    "CWR",
    "TCP CWR flag",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_NONCE = {
    "NONCE",
    "TCP nonce",
    Bit
};
inline constexpr ParameterSyntax PAR_TCP_PAYLOAD = {
    "payload",
    "TCP packet payload",
    Bytestream | Nested
};
inline constexpr ParameterSyntax PAR_TCP_CHKSUM = {
    "chksum",
    "TCP checksum",
    Int16
};
inline constexpr ParameterSyntaxArray PAR_TCP4_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    &PAR_TCP_SPORT,
    &PAR_TCP_DPORT,
    &PAR_TCP_SEQ,
    &PAR_TCP_ACK,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_TCP4_OPTIONAL = {
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    &PAR_TCP_WIN,
    &PAR_TCP_URGPTR,
    &PAR_TCP_FIN,
    &PAR_TCP_SYN,
    &PAR_TCP_RESET,
    &PAR_TCP_PUSH,
    &PAR_TCP_ACKFLAG,
    &PAR_TCP_URGENT,
    &PAR_TCP_ECN,
    &PAR_TCP_CWR,
    &PAR_TCP_NONCE,
    &PAR_TCP_PAYLOAD,
    &PAR_TCP_CHKSUM,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_TCP4 = {
    "tcp",
    "Raw TCPv4 packet",
    PAR_TCP4_MANDATORY,
    PAR_TCP4_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_TCP6_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP6,
    &PAR_TCP_SPORT,
    &PAR_TCP_DPORT,
    &PAR_TCP_SEQ,
    &PAR_TCP_ACK,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_TCP6_OPTIONAL = {
    &PAR_ETH_SMAC,
    PAR_IP6_OPT,
    &PAR_TCP_WIN,
    &PAR_TCP_URGPTR,
    &PAR_TCP_FIN,
    &PAR_TCP_SYN,
    &PAR_TCP_RESET,
    &PAR_TCP_PUSH,
    &PAR_TCP_ACKFLAG,
    &PAR_TCP_URGENT,
    &PAR_TCP_ECN,
    &PAR_TCP_CWR,
    &PAR_TCP_NONCE,
    &PAR_TCP_PAYLOAD,
    &PAR_TCP_CHKSUM,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_TCP6 = {
    "tcp6",
    "Raw TCPv6 packet",
    PAR_TCP6_MANDATORY,
    PAR_TCP6_OPTIONAL
};


inline constexpr ParameterSyntax PAR_VXLAN_VNI = {
    "vni",
    "VXLAN Network Identifier",
    Integer,
    "0",
    "16777215"
};
inline constexpr ParameterSyntax PAR_VXLAN_PAYLOAD = {
    "payload",
    "VXLAN payload data",
    Bytestream | Nested
};
inline constexpr ParameterSyntaxArray PAR_VXLAN4_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    &PAR_UDP_SPORT,
    &PAR_UDP_DPORT,
    &PAR_VXLAN_VNI,
    &PAR_VXLAN_PAYLOAD,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_VXLAN4_OPTIONAL = {
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_VXLAN4 = {
    "vxlan",
    "IPv4 Virtual eXtensible Local Area Network",
    PAR_VXLAN4_MANDATORY,
    PAR_VXLAN4_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_VXLAN6_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP6,
    &PAR_UDP_SPORT,
    &PAR_UDP_DPORT,
    &PAR_VXLAN_VNI,
    &PAR_VXLAN_PAYLOAD,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_VXLAN6_OPTIONAL = {
    &PAR_ETH_SMAC,
    PAR_IP6_OPT,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_VXLAN6 = {
    "vxlan6",
    "IPv6 Virtual eXtensible Local Area Network",
    PAR_VXLAN6_MANDATORY,
    PAR_VXLAN6_OPTIONAL
};


inline constexpr ParameterSyntax PAR_GRE_PROTOCOL = PAR_IP_PROTOCOL;
inline constexpr ParameterSyntax PAR_GRE_KEY = {
    "key",
    "GRE key",
    Int32
};
inline constexpr ParameterSyntax PAR_GRE_SEQ = {
    "seq",
    "GRE sequence number",
    Int32
};
inline constexpr ParameterSyntax PAR_GRE_CHKSUM = {
    "chksum",
    "GRE checksum",
    Int16
};
inline constexpr ParameterSyntax PAR_GRE_PAYLOAD = {
    "payload",
    "GRE payload data",
    Bytestream | Nested
};
inline constexpr ParameterSyntaxArray PAR_GRE4_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP4,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_GRE4_OPTIONAL = {
    &PAR_ETH_SMAC,
    PAR_IP4_OPT,
    &PAR_GRE_KEY,
    &PAR_GRE_SEQ,
    &PAR_GRE_CHKSUM,
    &PAR_GRE_PAYLOAD,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_GRE4 = {
    "gre",
    "IPv4 Generic Routing Encapsulation",
    PAR_GRE4_MANDATORY,
    PAR_GRE4_OPTIONAL
};

inline constexpr ParameterSyntaxArray PAR_GRE6_MANDATORY = {
    &PAR_ETH_DMAC,
    PAR_IP6,
    nullptr
};
inline constexpr ParameterSyntaxArray PAR_GRE6_OPTIONAL = {
    &PAR_ETH_SMAC,
    PAR_IP6_OPT,
    &PAR_GRE_KEY,
    &PAR_GRE_SEQ,
    &PAR_GRE_CHKSUM,
    &PAR_GRE_PAYLOAD,
    PAR_VLAN,
    nullptr
};
inline constexpr ProtocolSyntax PR_GRE6 = {
    "gre6",
    "IPv6 Generic Routing Encapsulation",
    PAR_GRE6_MANDATORY,
    PAR_GRE6_OPTIONAL
};


inline constexpr ParameterSyntax PAR_LLDP_CHASSIS_ID = {
    "chassis-id",
    "Chassis ID",
    Bytestream | IP4 | IP6 | Mac,
    "0",
    "255"
};
inline constexpr ParameterSyntax PAR_LLDP_CHASSIS_ID_T = {
    "chassis-id-type",
    "Chassis ID Subtype: 1 = chassis component, 2 = interface alias, 3 = port component, 4 = MAC, 5 = network address, 6 = interface name, 7 = local",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_PORT_ID = {
    "port-id",
    "Port ID",
    Bytestream | IP4 | IP6 | Mac,
    "0",
    "255"
};
inline constexpr ParameterSyntax PAR_LLDP_PORT_ID_T = {
    "port-id-type",
    "Port ID Subtype: 1 = interface alias, 2 = port component, 3 = MAC, 4 = network address, 5 = interface name, 6 = agent circuit ID, 7 = local",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_TTL = {
    "ttl",
    "Time To Live",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_PORT_DESC = {
    "port-desc",
    "Port Description",
    Bytestream,
    "0",
    "255"
};
inline constexpr ParameterSyntax PAR_LLDP_SYSNAME = {
    "sys-name",
    "System Name",
    Bytestream,
    "0",
    "255"
};
inline constexpr ParameterSyntax PAR_LLDP_SYSDESC = {
    "sys-desc",
    "System Description",
    Bytestream,
    "0",
    "255"
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_OTHER = {
    "cap-other",
    "System Capability 'Other'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_REPEATER = {
    "cap-repeater",
    "System Capability 'Repeater'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_BRIDGE = {
    "cap-bridge",
    "System Capability 'Bridge'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_WLAN = {
    "cap-wlan-ap",
    "System Capability 'WLAN AP'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_ROUTER = {
    "cap-router",
    "System Capability 'Router'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_PHONE = {
    "cap-phone",
    "System Capability 'Telephone'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_DOCSIS = {
    "cap-docsis",
    "System Capability 'DOCSIS cable device'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_STATION = {
    "cap-station",
    "System Capability 'Station only'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_CVLAN = {
    "cap-cvlan",
    "System Capability 'C-VLAN component'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_SVLAN = {
    "cap-svlan",
    "System Capability 'S-VLAN component'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_2P_RELAY = {
    "cap-tpmr",
    "System Capability 'Two-port MAC Relay component'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_OTHER_EN = {
    "encap-other",
    "Enabled System Capability 'Other'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_REPEATER_EN = {
    "encap-repeater",
    "Enabled System Capability 'Repeater'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_BRIDGE_EN = {
    "encap-bridge",
    "Enabled System Capability 'Bridge'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_WLAN_EN = {
    "encap-wlan-ap",
    "Enabled System Capability 'WLAN AP'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_ROUTER_EN = {
    "encap-router",
    "Enabled System Capability 'Router'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_PHONE_EN = {
    "encap-phone",
    "Enabled System Capability 'Telephone'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_DOCSIS_EN = {
    "encap-docsis",
    "Enabled System Capability 'DOCSIS cable device'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_STATION_EN = {
    "encap-station",
    "Enabled System Capability 'Station only'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_CVLAN_EN = {
    "encap-cvlan",
    "Enabled System Capability 'C-VLAN component'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_SVLAN_EN = {
    "encap-svlan",
    "Enabled System Capability 'S-VLAN component'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_SYSCAP_2P_RELAY_EN = {
    "encap-tpmr",
    "Enabled System Capability 'Two-port MAC Relay component'",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_MGT_ADDR = {
    "mgt-addr",
    "Management Address",
    Bytestream | IP4 | IP6 | Mac,
    "0",
    "31"
};
inline constexpr ParameterSyntax PAR_LLDP_MGT_ADDR_T = {
    "mgt-addr-type",
    "Management Address Subtype (see ianaAddressFamilyNumbers of RFC 3232 )",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_IF_NUMBER = {
    "if-number",
    "Interface Number",
    Int32
};
inline constexpr ParameterSyntax PAR_LLDP_IF_NUMBER_T = {
    "if-number-type",
    "Interface Number Subtype: 1 = unknown, 2 = ifIndex, 3 = system port number",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_MGT_OID = {
    "mgt-oid",
    "Management Object Identifier",
    Bytestream,
    "0",
    "128"
};
// Port VLAN ID TLV (IEEE 802.1Q-2022 D.2.1)
inline constexpr ParameterSyntax PAR_LLDP_PVID = {
    "pvid",
    "Port VLAN ID",
    Int16
};
// Port And ProtocolSyntax VLAN TLV (IEEE 802.1Q-2022 D.2.2)
inline constexpr ParameterSyntax PAR_LLDP_PPVID = {
    "ppvid",
    "Port and ProtocolSyntax VLAN ID",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_PPVID_SUP = {
    "PPVID-SUP",
    "Port and ProtocolSyntax VLAN supported",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_PPVID_EN = {
    "PPVID-EN",
    "Port and ProtocolSyntax VLAN enabled",
    Bit
};
// VLAN Name TLV (IEEE 802.1Q-2022 D.2.3)
inline constexpr ParameterSyntax PAR_LLDP_VLAN_NAME = {
    "vlan-name",
    "VLAN name",
    Bytestream,
    "0",
    "32"
};
inline constexpr ParameterSyntax PAR_LLDP_VLAN_NAME_VID = {
    "vlan-name-id",
    "VLAN ID of given name",
    Int16
};
// ProtocolSyntax Identity TLV (IEEE 802.1Q-2022 D.2.4)
inline constexpr ParameterSyntax PAR_LLDP_PROTO_ID = {
    "proto-id",
    "ProtocolSyntax Identity",
    Bytestream,
    "0",
    "255"
};
// VID Usage Digest TLV (IEEE 802.1Q-2022 D.2.5)
inline constexpr ParameterSyntax PAR_LLDP_VID_USAGE_DIGEST = {
    "vid-usage-digest",
    "VID usage digest",
    Int32
};
// Management VID TLV (IEEE 802.1Q-2022 D.2.6)
inline constexpr ParameterSyntax PAR_LLDP_MGT_VID = {
    "mgt-vid",
    "Management VID associated with the system",
    Int16
};
// Link Aggregation TLV (IEEE 802.1AX- F.2)
inline constexpr ParameterSyntax PAR_LLDP_LAG_CAP = {
    "lag-cap",
    "Link aggregation capability (0 = not capable, 1 = capable)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_LAG_STATUS = {
    "lag-status",
    "Link aggregation status (0 = not currently in aggregation, 1 = currently in aggregation)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_LAG_PORT_TYPE = {
    "lag-port-type",
    "Aggregation Port Type (0 = no port type, 1 = Aggregation Port, 2 = Aggregator, 3 = Aggregator with single port)",
    Integer,
    "0",
    "3"
};
inline constexpr ParameterSyntax PAR_LLDP_LAG_PORT_ID = {
    "lag-port-id",
    "Aggregated Port ID",
    Int32
};
// Congestion Notification TLV (IEEE 802.1Q-2022 D.2.7)
inline constexpr ParameterSyntax PAR_LLDP_CONG_NOTE_CNPV = {
    "cong-cnpv",
    "Per-priority CNPV indicators",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_CONG_NOTE_READY = {
    "cong-ready",
    "Per-priority Ready indicators",
    Int8
};
// ETS Configuration TLV (IEEE 802.1Q-2022 D.2.8)
inline constexpr ParameterSyntax PAR_LLDP_ETS_CFG_W = {
    "ets-cfg-willing",
    "Willing bit, if set, station accepts configurations",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_ETS_CFG_CBS = {
    "ets-cfg-cbs",
    "Credit-based Shaper bit, if set, station supports CBS",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_ETS_CFG_MAX_TC = {
    "ets-cfg-max-tc",
    "Maximum number of traffic classes supported (0 = 8 TCs)",
    Integer,
    "0",
    "7"
};
inline constexpr ParameterSyntax PAR_LLDP_ETS_CFG_PRIO = {
    "ets-cfg-prio",
    "Priority Assignment Table",
    Int32
};
inline constexpr ParameterSyntax PAR_LLDP_ETS_CFG_BW = {
    "ets-cfg-bw",
    "TC Bandwidth Table",
    Int64
};
inline constexpr ParameterSyntax PAR_LLDP_ETS_CFG_TSA = {
    "ets-cfg-tsa",
    "TSA Assignment Table",
    Int64
};
// ETS Recommendation TLV (IEEE 802.1Q-2022 D.2.9)
inline constexpr ParameterSyntax PAR_LLDP_ETS_REC_PRIO = {
    "ets-rec-prio",
    "Priority Assignment Table",
    Int32
};
inline constexpr ParameterSyntax PAR_LLDP_ETS_REC_BW = {
    "ets-rec-bw",
    "TC Bandwidth Table",
    Int64
};
inline constexpr ParameterSyntax PAR_LLDP_ETS_REC_TSA = {
    "ets-rec-tsa",
    "TSA Assignment Table",
    Int64
};
// Priority-based Flow Control Configuration TLV (IEEE 802.1Q-2022 D.2.10)
inline constexpr ParameterSyntax PAR_LLDP_PFC_W = {
    "pfc-willing",
    "Willing bit, if set, station accepts configurations",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_PFC_MBC = {
    "pfc-mbc",
    "MACsec Bypass Capability",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_PFC_CAP = {
    "pfc-cap",
    "PFC Capability",
    Integer,
    "0",
    "15"
};
inline constexpr ParameterSyntax PAR_LLDP_PFC_ENABLE = {
    "pfc-enable",
    "PFC Enable bit vector",
    Int8
};
// Application Priority TLV (IEEE 802.1Q-2022 D.2.11)
inline constexpr ParameterSyntax PAR_LLDP_APPL_PRIO = {
    "appl-prio",
    "Priority",
    Integer,
    "0",
    "7"
};
inline constexpr ParameterSyntax PAR_LLDP_APPL_SEL = {
    "appl-prio-sel",
    "Meaning of the protocol ID (1 = Ethertype, 2 = TCP/SCTP port, 3 = UDP port, 4 = UDP/TCP/SCTP/DCCP port, 5 = DSCP)",
    Integer,
    "0",
    "7"
};
inline constexpr ParameterSyntax PAR_LLDP_APPL_PROTO = {
    "appl-prio-proto",
    "ProtocolSyntax ID",
    Int16
};
// EVB TLV (IEEE 802.1Q-2022 D.2.12)
inline constexpr ParameterSyntax PAR_LLDP_EVB_BRIDGE_STATUS = {
    "evb-bridge-status",
    "EVB capabilities that are supported by the EVB bridge",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_EVB_STATION_STATUS = {
    "evb-station-status",
    "EVB capabilities that are supported by the EVB station",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_EVB_RETRIES = {
    "evb-max-retries",
    "maxRetries value for the ECP state machine",
    Integer,
    "0",
    "7"
};
inline constexpr ParameterSyntax PAR_LLDP_EVB_RTE = {
    "evb-rte",
    "Retransmission exponent",
    Integer,
    "0",
    "31"
};
inline constexpr ParameterSyntax PAR_LLDP_EVB_MODE = {
    "evb-mode",
    "EVB mode",
    Integer,
    "0",
    "3"
};
inline constexpr ParameterSyntax PAR_LLDP_EVB_ROL_RWD = {
    "evb-rol-rwd",
    "Remote or Local flag for RWD value (0 = local, 1 = remote)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_EVB_RWD = {
    "evb-rwd",
    "RWD value transmitted by the EVB bridge",
    Integer,
    "0",
    "31"
};
inline constexpr ParameterSyntax PAR_LLDP_EVB_ROL_RKA = {
    "evb-rol-rka",
    "Remote or Local flag for RKA value (0 = local, 1 = remote)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_EVB_RKA = {
    "evb-rka",
    "RKA value transmitted by the EVB station",
    Integer,
    "0",
    "31"
};
// CDCP TLV (IEEE 802.1Q-2022 D.2.13)
inline constexpr ParameterSyntax PAR_LLDP_CDCP_ROLE = {
    "cdcp-role",
    "Role (0 = Bridge, 1 = Station)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_CDCP_SCOMP = {
    "cdcp-scomp",
    "Presence of S-VLAN component for S-Channel",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_CDCP_CHN_CAP = {
    "cdcp-ch-cap",
    "Channel capacity",
    Integer,
    "0",
    "4095"
};
inline constexpr ParameterSyntax PAR_LLDP_CDCP_SCID = {
    "cdcp-scid",
    "Index number of S-channel",
    Integer,
    "0",
    "4095"
};
inline constexpr ParameterSyntax PAR_LLDP_CDCP_SVID = {
    "cdcp-svid",
    "VID assigned to the S-channel",
    Integer,
    "0",
    "4095"
};
// Application VLAN TLV (IEEE 802.1Q-2022 D.2.14)
inline constexpr ParameterSyntax PAR_LLDP_APPL_VLAN_VID = {
    "appl-vlan-vid",
    "VLAN ID",
    Integer,
    "0",
    "4095"
};
inline constexpr ParameterSyntax PAR_LLDP_APPL_VLAN_SEL = {
    "appl-vlan-sel",
    "Meaning of the protocol ID (1 = Ethertype, 2 = TCP/SCTP port, 3 = UDP port, 4 = UDP/TCP/SCTP/DCCP port, 5 = DSCP)",
    Integer,
    "0",
    "7"
};
inline constexpr ParameterSyntax PAR_LLDP_APPL_VLAN_PROTO = {
    "appl-vlan-proto",
    "ProtocolSyntax ID",
    Int16
};

// MAC/PHY Configuration/Status TLV (IEEE802.3-2022 clause 79.3.1)
inline constexpr ParameterSyntax PAR_LLDP_MACPHY_ANEG_SUP = {
    "autoneg-sup",
    "Auto-negotiation support",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_MACPHY_ANEG_ENA = {
    "autoneg-en",
    "Auto-negotiation enabled",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_MACPHY_ANEG_CAPS = {
    "autoneg-caps",
    "PMD auto-negotiation advertised capability",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_MACPHY_MAU_TYPE = {
    "mautype",
    "operational MAU type",
    Int16
};
// Power Via MDI TLV (IEEE802.3-2022 clause 79.3.2)
//  basic fields
inline constexpr ParameterSyntax PAR_LLDP_POE_MDI_POWER_SUP_PORT_CLASS = {
    "poe-port-class",
    "Port class (0 = PD, 1 = PSE)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_POE_MDI_POWER_SUP_PSE_MDI_SUP = {
    "poe-power-sup",
    "PSE MDI power support (0 = supported, 1 = not supported)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_POE_MDI_POWER_SUP_PSE_MDI_ENA = {
    "poe-power-state",
    "PSE MDI power state (0 = disabled, 1 = not enabled)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_POE_MDI_POWER_SUP_PSE_PAIR_CTRL = {
    "poe-pair-ctrl",
    "PSE pairs control ability (0 = pair selection can not be controlled, 1 = pair selection can be controlled)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_POE_PSE_POWER_PAIR = {
    "poe-power-pair",
    "PSE power pair field (1 = signal, 2 = spare)",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_POE_POWER_CLASS = {
    "poe-power-class",
    "Power class (1 = Class 0 PD, 2 = Class 1 PD, ... , 5 = Class 4 and above PD)",
    Int8
};
//  DLL classification extension
inline constexpr ParameterSyntax PAR_LLDP_POE_DLL_POWER_TYPE = {
    "poe-power-type",
    "DLL power type (0 = Type 2 PSE, 1 = Type 2 PD, 2 = Type 1 PSE, 3 = Type 1 PD)",
    Integer,
    "0",
    "3"
};
inline constexpr ParameterSyntax PAR_LLDP_POE_DLL_POWER_SOURCE = {
    "poe-power-src",
    "DLL power source (Power type = PSE: 0 = unknown, 1 = primary, 2 = backup | Power type = PD: 0 = unknown, 1 = PSE, 3 = PSE and local)",
    Integer,
    "0",
    "3"
};
inline constexpr ParameterSyntax PAR_LLDP_POE_DLL_PD_4PID = {
    "poe-pd-4pid",
    "PD 4PID (1 = PD supports powering of both Modes simultaneously, 0 = PD does not support...)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_POE_DLL_POWER_PRIO = {
    "poe-power-prio",
    "DLL power priority (0 = unknown, 1 = critical, 2 = high, 3 = low)",
    Integer,
    "0",
    "3"
};
inline constexpr ParameterSyntax PAR_LLDP_POE_DLL_PD_REQ_POWER = {
    "poe-req-power",
    "PD requested power value",
    Float,
    "0.0",
    "6553.5"
};
inline constexpr ParameterSyntax PAR_LLDP_POE_DLL_PD_ALLOC_POWER = {
    "poe-alloc-power",
    "PSE allocated power value",
    Float,
    "0.0",
    "6553.5"
};
// TODO Type 3 and Type 4 extension (~14 parameters!!!)

// Maximum Frame Size TLV (IEEE802.3-2022 clause 79.3.4)
inline constexpr ParameterSyntax PAR_LLDP_MAX_FRAME_SIZE = {
    "max-frame-size",
    "Maximum 802.3 frame size",
    Int16
};
// EEE TLV (IEEE802.3-2022 clause 79.3.5)
inline constexpr ParameterSyntax PAR_LLDP_EEE_TX_TW = {
    "eee-tx-tw",
    "EEE transmit Tw",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_EEE_RX_TW = {
    "eee-rx-tw",
    "EEE receive Tw",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_EEE_FB_RX_TW = {
    "eee-fb-rx-tw",
    "EEE fallback receive Tw",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_EEE_ECHO_TX_TW = {
    "eee-echo-tx-tw",
    "EEE echo transmit Tw",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_EEE_ECHO_RX_TW = {
    "eee-echo-rx-tw",
    "EEE echo receive  Tw",
    Int16
};
// EEE Fast Wake TLV (IEEE802.3-2022 clause 79.3.6)
inline constexpr ParameterSyntax PAR_LLDP_EEE_FW_TX = {
    "eee-fw-tx",
    "Transmit fast wake",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_EEE_FW_RX = {
    "eee-fw-rx",
    "Receive fast wake",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_EEE_FW_ECHO_TX = {
    "eee-fw-echo-tx",
    "Echo transmit fast wake",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_EEE_FW_ECHO_RX = {
    "eee-fw-echo-rx",
    "Echo receive fast wake",
    Int8
};

// Profinet TLV LLDP_PNIO_DELAY
inline constexpr ParameterSyntax PAR_LLDP_PN_DELAY_PORT_RX_LOC = {
    "pn-port-delay-rx",
    "PTCP_PortRxDelayLocal (nanoseconds)",
    Int32
};
inline constexpr ParameterSyntax PAR_LLDP_PN_DELAY_PORT_RX_REM = {
    "pn-port-delay-rx-rem",
    "PTCP_PortRxDelayRemote (nanoseconds)",
    Int32
};
inline constexpr ParameterSyntax PAR_LLDP_PN_DELAY_PORT_TX_LOC = {
    "pn-port-delay-tx",
    "PTCP_PortTxDelayLocal (nanoseconds)",
    Int32
};
inline constexpr ParameterSyntax PAR_LLDP_PN_DELAY_PORT_TX_REM = {
    "pn-port-delay-tx-rem",
    "PTCP_PortTxDelayRemote (nanoseconds)",
    Int32
};
inline constexpr ParameterSyntax PAR_LLDP_PN_DELAY_LINE = {
    "pn-cable-delay",
    "Measured cable delay (nanoseconds)",
    Int32
};
// Profinet TLV LLDP_PNIO_PORTSTATUS
inline constexpr ParameterSyntax PAR_LLDP_PN_RTC2_STATE = {
    "pn-rtc2-state",
    "RTClass2_PortStatus.State (0 = OFF, 1 = SYNC-DATA-LOADED, 2 = UP)",
    Integer,
    "0",
    "3"
};
inline constexpr ParameterSyntax PAR_LLDP_PN_RTC3_STATE = {
    "pn-rtc3-state",
    "RTClass3_PortStatus.State (0 = OFF, 2 = UP, 4 = RUN)",
    Integer,
    "0",
    "7"
};
inline constexpr ParameterSyntax PAR_LLDP_PN_RTC3_FRAG = {
    "pn-rtc3-frag",
    "Fragmentation Mode (0 = disabled, 1 = enabled)",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_PN_RTC3_PREAMP = {
    "pn-rtc3-short-preamp",
    "Short preample (0 = disabled (8 octets), 1 = enabled (1 octet))",
    Bit
};
inline constexpr ParameterSyntax PAR_LLDP_PN_RTC3_OPTIMIZED = {
    "pn-rtc3-opt",
    "Optimized (0 = OFF, 1 = ON)",
    Bit
};
// Profinet TLV LLDP_PNIO_ALIAS
inline constexpr ParameterSyntax PAR_LLDP_PN_ALIAS = {
    "pn-alias",
    "Alias name value",
    Bytestream,
    "0",
    "255"
};
// Profinet TLV LLDP_PNIO_MRPPORTSTATUS
inline constexpr ParameterSyntax PAR_LLDP_PN_MRP_DOMAIN = {
    "pn-mrp-domain",
    "MRP domain name",
    Bytestream
};
inline constexpr ParameterSyntax PAR_LLDP_PN_MRP_DOMAIN_UUID = {
    "pn-mrp-domain-uuid",
    "MRP domain uuid",
    UUID
};
inline constexpr ParameterSyntax PAR_LLDP_PN_MRP_MRRT_STATE = {
    "pn-mrp-mrrt-state",
    "MRRT port status (0 = OFF, 1 = CONFIGURED, 2 = UP)",
    Integer,
    "0",
    "3"
};
// Profinet TLV LLDP_PNIO_CHASSIS_MAC
inline constexpr ParameterSyntax PAR_LLDP_PN_CHASSIS_MAC = {
    "pn-chassis-mac",
    "Chassis MAC address",
    Mac
};
// Profinet TLV LLDP_PNIO_PTCPSTATUS
inline constexpr ParameterSyntax PAR_LLDP_PN_PTCP_MAST_SRC_MAC = {
    "pn-ptcp-master-mac",
    "PTCP master source MAC address",
    Mac
};
inline constexpr ParameterSyntax PAR_LLDP_PN_PTCP_DOMAIN_UUID = {
    "pn-ptcp-domain-uuid",
    "PTCP domain UUID",
    UUID
};
inline constexpr ParameterSyntax PAR_LLDP_PN_PTCP_IRDATA_UUID = {
    "pn-ptcp-irdata-uuid",
    "IRDATA UUID",
    UUID
};
inline constexpr ParameterSyntax PAR_LLDP_PN_PTCP_PERIOD_LEN = {
    "pn-ptcp-period-len",
    "Length of period (nanoseconds)",
    Integer,
    "0",
    "2147483647"
};
inline constexpr ParameterSyntax PAR_LLDP_PN_PTCP_RED_ORANGE = {
    "pn-ptcp-red-orange",
    "Frame offset of red/orange period (nanoseconds)",
    Integer,
    "0",
    "2147483647"
};
inline constexpr ParameterSyntax PAR_LLDP_PN_PTCP_ORANGE = {
    "pn-ptcp-orange",
    "Frame offset of orange period (nanoseconds)",
    Integer,
    "0",
    "2147483647"
};
inline constexpr ParameterSyntax PAR_LLDP_PN_PTCP_GREEN = {
    "pn-ptcp-green",
    "Frame offset of green period (nanoseconds)",
    Integer,
    "0",
    "2147483647"
};
// Profinet TLV LLDP_PNIO_MAUTypeExtension
inline constexpr ParameterSyntax PAR_LLDP_PN_MAU_TYPE_EXT = {
    "pn-mautype-ext",
    "MAUTYPE extension",
    Int16
};
// Profinet TLV LLDP_PNIO_MRPICPORT_STATUS
inline constexpr ParameterSyntax PAR_LLDP_PN_MRP_IC_DOMAIN_ID = {
    "pn-mrp-ic-domain-id",
    "MRP interconnection domain identifier",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_PN_MRP_IC_ROLE = {
    "pn-mrp-ic-role",
    "MRP interconnection role (0 = none, 1 = client, 2 = manager)",
    Int16
};
inline constexpr ParameterSyntax PAR_LLDP_PN_MRP_IC_MIC_POS = {
    "pn-mrp-ic-mic-pos",
    "MRP interconnection mic position (0 = Primary, 1 = Secondary)",
    Int16
};

// Raw user defined TLV
inline constexpr ParameterSyntax PAR_LLDP_TLV_TYPE = {
    "type",
    "Raw TLV Type Number",
    Integer,
    "0",
    "127"
};
inline constexpr ParameterSyntax PAR_LLDP_TLV_VALUE = {
    "value",
    "Raw TLV Value as bytestream",
    Bytestream,
    "0",
    "511"
};
// Raw user defined organizationally specific TLV
inline constexpr ParameterSyntax PAR_LLDP_OUI_TLV_OUI = {
    "oui",
    "Organizationally Specific TLV OUI",
    Bytestream,
    "3",
    "3"
};
inline constexpr ParameterSyntax PAR_LLDP_OUI_TLV_TYPE = {
    "oui-type",
    "Organizationally Specific TLV Subtype Number",
    Int8
};
inline constexpr ParameterSyntax PAR_LLDP_OUI_TLV_VALUE = {
    "oui-value",
    "Organizationally Specific TLV Value as bytestream",
    Bytestream,
    "0",
    "507"
};
inline constexpr ParameterSyntaxArray PAR_LLDP_OPTIONAL = {
    &PAR_ETH_DMAC,
    &PAR_ETH_SMAC,
    PAR_VLAN,
    &PAR_LLDP_CHASSIS_ID,
    &PAR_LLDP_CHASSIS_ID_T,
    &PAR_LLDP_PORT_ID,
    &PAR_LLDP_PORT_ID_T,
    &PAR_LLDP_TTL,
    &PAR_LLDP_PORT_DESC,
    &PAR_LLDP_SYSNAME,
    &PAR_LLDP_SYSDESC,
    &PAR_LLDP_SYSCAP_OTHER,
    &PAR_LLDP_SYSCAP_REPEATER,
    &PAR_LLDP_SYSCAP_BRIDGE,
    &PAR_LLDP_SYSCAP_WLAN,
    &PAR_LLDP_SYSCAP_ROUTER,
    &PAR_LLDP_SYSCAP_PHONE,
    &PAR_LLDP_SYSCAP_DOCSIS,
    &PAR_LLDP_SYSCAP_STATION,
    &PAR_LLDP_SYSCAP_OTHER_EN,
    &PAR_LLDP_SYSCAP_REPEATER_EN,
    &PAR_LLDP_SYSCAP_BRIDGE_EN,
    &PAR_LLDP_SYSCAP_WLAN_EN,
    &PAR_LLDP_SYSCAP_ROUTER_EN,
    &PAR_LLDP_SYSCAP_PHONE_EN,
    &PAR_LLDP_SYSCAP_DOCSIS_EN,
    &PAR_LLDP_SYSCAP_STATION_EN,
    &PAR_LLDP_MGT_ADDR,
    &PAR_LLDP_MGT_ADDR_T,
    &PAR_LLDP_IF_NUMBER,
    &PAR_LLDP_IF_NUMBER_T,
    &PAR_LLDP_MGT_OID,
    &PAR_LLDP_PVID,
    &PAR_LLDP_PPVID,
    &PAR_LLDP_PPVID_SUP,
    &PAR_LLDP_PPVID_EN,
    &PAR_LLDP_VLAN_NAME_VID,
    &PAR_LLDP_VLAN_NAME,
    &PAR_LLDP_PROTO_ID,
    &PAR_LLDP_VID_USAGE_DIGEST,
    &PAR_LLDP_MGT_VID,
    &PAR_LLDP_LAG_CAP,
    &PAR_LLDP_LAG_STATUS,
    &PAR_LLDP_LAG_PORT_TYPE,
    &PAR_LLDP_LAG_PORT_ID,
    &PAR_LLDP_CONG_NOTE_CNPV,
    &PAR_LLDP_CONG_NOTE_READY,
    &PAR_LLDP_ETS_CFG_W,
    &PAR_LLDP_ETS_CFG_CBS,
    &PAR_LLDP_ETS_CFG_MAX_TC,
    &PAR_LLDP_ETS_CFG_PRIO,
    &PAR_LLDP_ETS_CFG_BW,
    &PAR_LLDP_ETS_CFG_TSA,
    &PAR_LLDP_ETS_REC_PRIO,
    &PAR_LLDP_ETS_REC_BW,
    &PAR_LLDP_ETS_REC_TSA,
    &PAR_LLDP_PFC_W,
    &PAR_LLDP_PFC_MBC,
    &PAR_LLDP_PFC_CAP,
    &PAR_LLDP_PFC_ENABLE,
    &PAR_LLDP_APPL_PRIO,
    &PAR_LLDP_APPL_SEL,
    &PAR_LLDP_APPL_PROTO,
    &PAR_LLDP_EVB_BRIDGE_STATUS,
    &PAR_LLDP_EVB_STATION_STATUS,
    &PAR_LLDP_EVB_RETRIES,
    &PAR_LLDP_EVB_RTE,
    &PAR_LLDP_EVB_MODE,
    &PAR_LLDP_EVB_ROL_RWD,
    &PAR_LLDP_EVB_RWD,
    &PAR_LLDP_EVB_ROL_RKA,
    &PAR_LLDP_EVB_RKA,
    &PAR_LLDP_CDCP_ROLE,
    &PAR_LLDP_CDCP_SCOMP,
    &PAR_LLDP_CDCP_CHN_CAP,
    &PAR_LLDP_CDCP_SCID,
    &PAR_LLDP_CDCP_SVID,
    &PAR_LLDP_APPL_VLAN_VID,
    &PAR_LLDP_APPL_VLAN_SEL,
    &PAR_LLDP_APPL_VLAN_PROTO,

    &PAR_LLDP_MACPHY_ANEG_SUP,
    &PAR_LLDP_MACPHY_ANEG_ENA,
    &PAR_LLDP_MACPHY_ANEG_CAPS,
    &PAR_LLDP_MACPHY_MAU_TYPE,

    &PAR_LLDP_POE_MDI_POWER_SUP_PORT_CLASS,
    &PAR_LLDP_POE_MDI_POWER_SUP_PSE_MDI_SUP,
    &PAR_LLDP_POE_MDI_POWER_SUP_PSE_MDI_ENA,
    &PAR_LLDP_POE_MDI_POWER_SUP_PSE_PAIR_CTRL,
    &PAR_LLDP_POE_PSE_POWER_PAIR,
    &PAR_LLDP_POE_POWER_CLASS,
    &PAR_LLDP_POE_DLL_POWER_TYPE,
    &PAR_LLDP_POE_DLL_POWER_SOURCE,
    &PAR_LLDP_POE_DLL_PD_4PID,
    &PAR_LLDP_POE_DLL_POWER_PRIO,
    &PAR_LLDP_POE_DLL_PD_REQ_POWER,
    &PAR_LLDP_POE_DLL_PD_ALLOC_POWER,

    &PAR_LLDP_MAX_FRAME_SIZE,
    &PAR_LLDP_EEE_TX_TW,
    &PAR_LLDP_EEE_RX_TW,
    &PAR_LLDP_EEE_FB_RX_TW,
    &PAR_LLDP_EEE_ECHO_TX_TW,
    &PAR_LLDP_EEE_ECHO_RX_TW,
    &PAR_LLDP_EEE_FW_TX,
    &PAR_LLDP_EEE_FW_RX,
    &PAR_LLDP_EEE_FW_ECHO_TX,
    &PAR_LLDP_EEE_FW_ECHO_RX,

    &PAR_LLDP_PN_DELAY_PORT_RX_LOC,
    &PAR_LLDP_PN_DELAY_PORT_RX_REM,
    &PAR_LLDP_PN_DELAY_PORT_TX_LOC,
    &PAR_LLDP_PN_DELAY_PORT_TX_REM,
    &PAR_LLDP_PN_DELAY_LINE,
    &PAR_LLDP_PN_RTC2_STATE,
    &PAR_LLDP_PN_RTC3_STATE,
    &PAR_LLDP_PN_RTC3_FRAG,
    &PAR_LLDP_PN_RTC3_PREAMP,
    &PAR_LLDP_PN_RTC3_OPTIMIZED,
    &PAR_LLDP_PN_ALIAS,
    &PAR_LLDP_PN_MRP_DOMAIN,
    &PAR_LLDP_PN_MRP_DOMAIN_UUID,
    &PAR_LLDP_PN_MRP_MRRT_STATE,
    &PAR_LLDP_PN_CHASSIS_MAC,
    &PAR_LLDP_PN_PTCP_MAST_SRC_MAC,
    &PAR_LLDP_PN_PTCP_DOMAIN_UUID,
    &PAR_LLDP_PN_PTCP_IRDATA_UUID,
    &PAR_LLDP_PN_PTCP_PERIOD_LEN,
    &PAR_LLDP_PN_PTCP_RED_ORANGE,
    &PAR_LLDP_PN_PTCP_ORANGE,
    &PAR_LLDP_PN_PTCP_GREEN,
    &PAR_LLDP_PN_MAU_TYPE_EXT,
    &PAR_LLDP_PN_MRP_IC_DOMAIN_ID,
    &PAR_LLDP_PN_MRP_IC_ROLE,
    &PAR_LLDP_PN_MRP_IC_MIC_POS,

    &PAR_LLDP_TLV_TYPE,
    &PAR_LLDP_TLV_VALUE,
    &PAR_LLDP_OUI_TLV_OUI,
    &PAR_LLDP_OUI_TLV_TYPE,
    &PAR_LLDP_OUI_TLV_VALUE,
    nullptr
};
inline constexpr ProtocolSyntax PR_LLDP = {
    "lldp",
    "Link Layer Discovery ProtocolSyntax",
    empty_params,
    PAR_LLDP_OPTIONAL
};

inline constexpr std::array all_protos {
    &PR_RAW,
    &PR_ETH,
    &PR_ARP,
    &PR_ARP_PROBE,
    &PR_ARP_ANNOUNCE,
    &PR_IPV4,
    &PR_IPV6,
    &PR_UDP4,
    &PR_UDP6,
    &PR_VRRP,
    &PR_VRRP3,
    &PR_STP,
    &PR_RSTP,
    &PR_STP_TCN,
    &PR_IGMP,
    &PR_IGMP_QUERY,
    &PR_IGMP3_QUERY,
    &PR_IGMP_REPORT,
    &PR_IGMP_LEAVE,
    &PR_ICMP4,
    &PR_ICMP4_UNREACH,
    &PR_ICMP4_SRCQ,
    &PR_ICMP4_TIMEX,
    &PR_ICMP4_REDIR,
    &PR_ICMP4_ECHO,
    &PR_ICMP4_ECHOR,
    &PR_TCP4,
    &PR_TCP6,
    &PR_VXLAN4,
    &PR_VXLAN6,
    &PR_GRE4,
    &PR_GRE6,
    &PR_LLDP
};

#endif
