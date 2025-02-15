// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2025 Andreas Martin (netnag@mailbox.org)
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

#include <vector>


enum Type {Integer, Float, Mac, IP, Bytestream, Bit};

struct Parameter
{
    const char* syntax;
    const char* descr;
    const Type type;
};

struct Protocol
{
    const char* syntax;
    const char* descr;
    const std::vector<const Parameter*> mandatory;
    const std::vector<const Parameter*> optional;
};


static const Parameter PAR_RAW_BYTE = {
    "byte",
    "Raw byte value",
    Integer
};
static const Parameter PAR_RAW_BE16 = {
    "be16",
    "Big-endian 16-bit value",
    Integer
};
static const Parameter PAR_RAW_BE32 = {
    "be32",
    "Big-endian 32-bit value",
    Integer
};
static const Parameter PAR_RAW_BE64 = {
    "be64",
    "Big-endian 64-bit value",
    Integer
};
static const Parameter PAR_RAW_LE16 = {
    "le16",
    "Little-endian 16-bit value",
    Integer
};
static const Parameter PAR_RAW_LE32 = {
    "le32",
    "Little-endian 32-bit value",
    Integer
};
static const Parameter PAR_RAW_LE64 = {
    "le64",
    "Little-endian 64-bit value",
    Integer
};
static const Parameter PAR_RAW_IP4 = {
    "ip4",
    "IPv4 address",
    IP
};
static const Parameter PAR_RAW_IP6 = {
    "ip6",
    "IPv6 address",
    IP
};
static const Parameter PAR_RAW_MAC = {
    "mac",
    "EUI-48 Mac address",
    Mac
};
static const Parameter PAR_RAW_STREAM = {
    "stream",
    "Data stream",
    Bytestream
};
static const Protocol PR_RAW = {
    "raw",
    "raw custom packet",
    {},
    {
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
        &PAR_RAW_STREAM
    }
};


static const Parameter PAR_ETH_SMAC = {
    "smac",
    "Source EUI-48 Mac address",
    Mac
};
static const Parameter PAR_ETH_DMAC = {
    "dmac",
    "Destination EUI-48 Mac address",
    Mac
};
static const Parameter PAR_ETH_DSAP = {
    "dsap",
    "IEEE 802.2 DSAP field",
    Integer
};
static const Parameter PAR_ETH_SSAP = {
    "ssap",
    "IEEE 802.2 SSAP field",
    Integer
};
static const Parameter PAR_ETH_CONTROL = {
    "control",
    "Control field",
    Integer
};
static const Parameter PAR_ETH_OUI = {
    "oui",
    "Organizationally Unique Identifier",
    Integer
};
static const Parameter PAR_ETH_PROTOCOL = {
    "protocol",
    "Protocol identifier",
    Integer
};
static const Parameter PAR_ETH_PAYLOAD = {
    "payload",
    "Ethernet payload data",
    Bytestream
};
static const Parameter PAR_ETH_ETHERTYPE = {
    "ethertype",
    "EtherType field",
    Integer
};
static const Parameter PAR_ETH_VID = {
    "vid",
    "VLAN Identifier",
    Integer
};
static const Parameter PAR_ETH_VTYPE = {
    "vtype",
    "VLAN Type",
    Integer
};
static const Parameter PAR_ETH_PRIO = {
    "prio",
    "VLAN Priority",
    Integer
};
static const Parameter PAR_ETH_DEI = {
    "dei",
    "Drop Eligible Indicator",
    Bit
};
// shortcut for VLAN tag parameters
#define PAR_VLAN &PAR_ETH_VID, &PAR_ETH_VTYPE, &PAR_ETH_PRIO, &PAR_ETH_DEI
static const Protocol PR_ETH = {
    "eth",
    "Ethernet II or IEEE802.3 packet",
    {
        &PAR_ETH_DMAC,
        &PAR_ETH_PAYLOAD
    },
    {
        &PAR_ETH_SMAC,
        &PAR_ETH_ETHERTYPE,
        PAR_VLAN,
        &PAR_ETH_DSAP,
        &PAR_ETH_SSAP,
        &PAR_ETH_CONTROL,
        &PAR_ETH_OUI,
        &PAR_ETH_PROTOCOL
    }
};


static const Parameter PAR_IP_DSCP = {
    "dscp",
    "Differentiated Services Code Point",
    Integer
};
static const Parameter PAR_IP_ECN = {
    "ecn",
    "Explicit Congestion Notification",
    Integer
};
static const Parameter PAR_IP_TTL = {
    "ttl",
    "Time To Live",
    Integer
};
static const Parameter PAR_IP_DIP = {
    "dip",
    "Destination IP address",
    IP
};
static const Parameter PAR_IP_SIP = {
    "sip",
    "Source IP address",
    IP
};
static const Parameter PAR_IP_PROTOCOL = {
    "protocol",
    "Transport layer protocol",
    Integer
};
static const Parameter PAR_IP_PAYLOAD = {
    "payload",
    "IP packet payload",
    Bytestream
};
static const Parameter PAR_IP4_ID = {
    "id",
    "IPv4 packet identifier",
    Integer
};
static const Parameter PAR_IP4_DF = {
    "df",
    "IPv4 Don't Fragment flag",
    Bit
};
static const Parameter PAR_IP6_FL = {
    "fl",
    "IPv6 Flow Label",
    Integer
};
// shortcuts for IP header parameters
#define PAR_IP4_OPT &PAR_IP_DSCP, &PAR_IP_ECN, &PAR_IP_TTL, &PAR_IP4_DF, &PAR_IP_SIP, &PAR_IP4_ID
#define PAR_IP4 &PAR_IP_DIP
#define PAR_IP6_OPT &PAR_IP_DSCP, &PAR_IP_ECN, &PAR_IP_TTL, &PAR_IP_SIP, &PAR_IP6_FL
#define PAR_IP6 &PAR_IP_DIP
static const Protocol PR_IPV4 = {
    "ipv4",
    "Raw IPv4 packet",
    {
        PAR_IP4,
        &PAR_IP_PROTOCOL,
        &PAR_IP_PAYLOAD
    },
    {
        &PAR_ETH_SMAC,
        &PAR_ETH_DMAC,
        PAR_IP4_OPT,
        PAR_VLAN
    }
};
static const Protocol PR_IPV6 = {
    "ipv6",
    "Raw IPv6 packet",
    {
        PAR_IP6,
        &PAR_IP_PROTOCOL,
        &PAR_IP_PAYLOAD
    },
    {
        &PAR_ETH_SMAC,
        &PAR_ETH_DMAC,
        PAR_IP6_OPT,
        PAR_VLAN
    }
};


static const Parameter PAR_UDP_SPORT = {
    "sport",
    "Source UDP port",
    Integer
};
static const Parameter PAR_UDP_DPORT = {
    "dport",
    "Destination UDP port",
    Integer
};
static const Parameter PAR_UDP_PAYLOAD = {
    "payload",
    "UDP packet payload",
    Bytestream
};
static const Parameter PAR_UDP_CHKSUM = {
    "chksum",
    "UDP checksum",
    Integer
};
static const Protocol PR_UDP4 = {
    "udp",
    "IPv4 User Datagram Protocol",
    {
        PAR_IP4,
        &PAR_UDP_SPORT,
        &PAR_UDP_DPORT,
    },
    {
        &PAR_ETH_SMAC,
        &PAR_ETH_DMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
        &PAR_UDP_PAYLOAD,
        &PAR_UDP_CHKSUM,
    }
};
static const Protocol PR_UDP6 = {
    "udp6",
    "IPv6 User Datagram Protocol",
    {
        PAR_IP6,
        &PAR_UDP_SPORT,
        &PAR_UDP_DPORT,

    },
    {
        &PAR_ETH_SMAC,
        &PAR_ETH_DMAC,
        PAR_IP6_OPT,
        PAR_VLAN,
        &PAR_UDP_PAYLOAD,
        &PAR_UDP_CHKSUM,
    }
};


static const Parameter PAR_ARP_OP = {
    "op",
    "Opcode, 1 = request, 2 = reply",
    Integer
};
static const Protocol PR_ARP = {
    "arp",
    "Raw ARP packet",
    {
        &PAR_ARP_OP,
        &PAR_IP_DIP,
    },
    {
        &PAR_ETH_SMAC,
        &PAR_ETH_DMAC,
        &PAR_IP_SIP,
        PAR_VLAN,
    }
};
static const Protocol PR_ARP_PROBE = {
    "arp-probe",
    "ARP probe packet",
    { &PAR_IP_DIP },
    { PAR_VLAN }
};
static const Protocol PR_ARP_ANNOUNCE = {
    "arp-announce",
    "ARP announce packet",
    { },
    { &PAR_IP_DIP, PAR_VLAN }
};


static const Parameter PAR_VRRP_VRIP = {
    "vrip",
    "Virtual Router IP address",
    IP
};
static const Parameter PAR_VRRP_VRID = {
    "vrid",
    "Virtual Router ID",
    Integer
};
static const Parameter PAR_VRRP_VRPRIO = {
    "vrprio",
    "Virtual Router Priority",
    Integer
};
static const Parameter PAR_VRRP_TYPE = {
    "type",
    "VRRP message type",
    Integer
};
static const Parameter PAR_VRRP_AINT = {
    "aint",
    "Advertisement Interval",
    Integer
};
static const Parameter PAR_VRRP_CHKSUM = {
    "chksum",
    "VRRP checksum",
    Integer
};
static const Protocol PR_VRRP = {
    "vrrp",
    "Virual Router Redundancy Protocol V2",
    {
        &PAR_VRRP_VRIP,
        &PAR_VRRP_VRID,
    },
    {
        &PAR_ETH_SMAC,
        &PAR_IP_SIP,
        &PAR_VRRP_VRPRIO,
        &PAR_VRRP_TYPE,
        &PAR_VRRP_AINT,
        &PAR_VRRP_CHKSUM,
        PAR_VLAN,
    }
};
static const Protocol PR_VRRP3 = {
    "vrrp3",
    "Virual Router Redundancy Protocol V3",
    {
        &PAR_VRRP_VRIP,
        &PAR_VRRP_VRID,
    },
    {
        &PAR_ETH_SMAC,
        &PAR_IP_SIP,
        &PAR_VRRP_VRPRIO,
        &PAR_VRRP_TYPE,
        &PAR_VRRP_AINT,
        &PAR_VRRP_CHKSUM,
        PAR_VLAN,
    }
};


static const Parameter PAR_STP_RBPRIO = {
    "rbprio",
    "Root Bridge Priority",
    Integer
};
static const Parameter PAR_STP_RBIDEXT = {
    "rbidext",
    "Root Bridge ID Extension",
    Integer
};
static const Parameter PAR_STP_RBMAC = {
    "rbmac",
    "Root Bridge EUI-48 Mac address",
    Mac
};
static const Parameter PAR_STP_BPRIO = {
    "bprio",
    "Bridge Priority",
    Integer
};
static const Parameter PAR_STP_BIDEXT = {
    "bidext",
    "Bridge ID Extension",
    Integer
};
static const Parameter PAR_STP_BMAC = {
    "bmac",
    "Bridge EUI-48 Mac address",
    Mac
};
static const Parameter PAR_STP_PPRIO = {
    "pprio",
    "Port Priority",
    Integer
};
static const Parameter PAR_STP_PNUM = {
    "pnum",
    "Port Number",
    Integer
};
static const Parameter PAR_STP_MSGAGE = {
    "msgage",
    "Message Age",
    Integer
};
static const Parameter PAR_STP_MAXAGE = {
    "maxage",
    "Max Age",
    Integer
};
static const Parameter PAR_STP_HELLO = {
    "hello",
    "Hello Time",
    Integer
};
static const Parameter PAR_STP_DELAY = {
    "delay",
    "Forward Delay",
    Integer
};
static const Parameter PAR_STP_TOPOCHANGE = {
    "topochange",
    "Topology Change",
    Bit
};
static const Parameter PAR_STP_TOPOCHANGEACK = {
    "topochangeack",
    "Topology Change Acknowledgement",
    Bit
};
static const Parameter PAR_STP_RPATHCOST = {
    "rpathcost",
    "Root Path Cost",
    Integer
};
static const Parameter PAR_STP_PORTROLE = {
    "portrole",
    "Port Role",
    Integer
};
static const Parameter PAR_STP_PROPOSAL = {
    "proposal",
    "Proposal",
    Bit
};
static const Parameter PAR_STP_LEARNING = {
    "learning",
    "Learning Mode",
    Bit
};
static const Parameter PAR_STP_FORWARDING = {
    "forwarding",
    "Forwarding Mode",
    Bit
};
static const Parameter PAR_STP_AGREEMENT = {
    "agreement",
    "Agreement",
    Bit
};
static const Protocol PR_STP = {
    "stp",
    "Spanning Tree Protocol",
    {},
    {
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
    }
};
static const Protocol PR_RSTP = {
    "rstp",
    "Rapid Spanning Tree Protocol",
    {},
    {
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
        &PAR_STP_PORTROLE,
        &PAR_STP_PROPOSAL,
        &PAR_STP_LEARNING,
        &PAR_STP_FORWARDING,
        &PAR_STP_AGREEMENT,
        PAR_VLAN
    }
};
static const Protocol PR_STP_TCN = {
    "stp-tcn",
    "STP Topology Change Notification",
    {},
    {}
};


static const Parameter PAR_IGMP_S = {
    "s",
    "Suppress Router-side Processing",
    Bit
};
static const Parameter PAR_IGMP_QRV = {
    "qrv",
    "Query Response Interval",
    Integer
};
static const Parameter PAR_IGMP_QQIC = {
    "qqic",
    "Querier's Query Interval Count",
    Float
};
static const Parameter PAR_IGMP_TIME = {
    "time",
    "IGMP Time",
    Float
};
static const Parameter PAR_IGMP_RSIP = {
    "rsip",
    "Router Source IP address",
    IP
};
static const Parameter PAR_IGMP_GROUP = {
    "group",
    "Multicast group address",
    IP
};
static const Parameter PAR_IGMP_TYPE = {
    "type",
    "IGMP message type",
    Integer
};
static const Protocol PR_IGMP = {
    "igmp",
    "Raw IGMP V1/V2 packet",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
        &PAR_IGMP_GROUP,
        &PAR_IGMP_TYPE
    },
    {
        &PAR_IGMP_TIME,
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN
    }
};
static const Protocol PR_IGMP_QUERY = {
    "igmp-query",
    "IGMP V1/V2 Query",
    {},
    {
        &PAR_IGMP_TIME,
        &PAR_IGMP_GROUP,
        PAR_IP4_OPT,
        PAR_VLAN
    }
};
static const Protocol PR_IGMP3_QUERY = {
    "igmp3-query",
    "IGMP V3 Query",
    {},
    {
        &PAR_IGMP_TIME,
        &PAR_IGMP_GROUP,
        &PAR_IGMP_S,
        &PAR_IGMP_QRV,
        &PAR_IGMP_QQIC,
        &PAR_IGMP_RSIP,
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};
static const Protocol PR_IGMP_REPORT = {
    "igmp-report",
    "IGMP V1/V2 Report",
    { &PAR_IGMP_GROUP },
    {
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};
static const Protocol PR_IGMP_LEAVE = {
    "igmp-leave",
    "IGMP V1/V2 Leave",
    { &PAR_IGMP_GROUP },
    {
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};


static const Parameter PAR_ICMP4_TYPE = {
    "type",
    "ICMPv4 message type",
    Integer
};
static const Parameter PAR_ICMP4_CODE = {
    "code",
    "ICMPv4 message code",
    Integer
};
static const Parameter PAR_ICMP4_PAYLOAD = {
    "payload",
    "ICMPv4 message payload",
    Bytestream
};
static const Parameter PAR_ICMP4_CHKSUM = {
    "chksum",
    "ICMPv4 checksum",
    Integer
};
static const Parameter PAR_ICMP4_GW = {
    "gw",
    "Gateway address",
    IP
};
static const Parameter PAR_ICMP4_ID = {
    "id",
    "ICMPv4 identifier",
    Integer
};
static const Parameter PAR_ICMP4_SEQ = {
    "seq",
    "ICMPv4 sequence number",
    Integer
};
static const Protocol PR_ICMP4 = {
    "icmp",
    "Raw ICMPv4 packet",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
        &PAR_ICMP4_TYPE,
        &PAR_ICMP4_CODE,
    },
    {
        &PAR_ICMP4_PAYLOAD,
        &PAR_ICMP4_CHKSUM,
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};
static const Protocol PR_ICMP4_UNREACH = {
    "icmp-unreachable",
    "ICMPv4 Unreachable",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
    },
    {
        &PAR_ICMP4_CODE,
        &PAR_ICMP4_PAYLOAD,
        &PAR_ICMP4_CHKSUM,
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};
static const Protocol PR_ICMP4_SRCQ = {
    "icmp-src-quench",
    "ICMPv4 Source Quench",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
    },
    {
        &PAR_ICMP4_CODE,
        &PAR_ICMP4_PAYLOAD,
        &PAR_ICMP4_CHKSUM,
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};
static const Protocol PR_ICMP4_TIMEX = {
    "icmp-time-exceeded",
    "ICMPv4 Time Exceeded",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
    },
    {
        &PAR_ICMP4_CODE,
        &PAR_ICMP4_PAYLOAD,
        &PAR_ICMP4_CHKSUM,
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
    },
};
static const Protocol PR_ICMP4_REDIR = {
    "icmp-redirect",
    "ICMPv4 Redirect",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
        &PAR_ICMP4_GW,
    },
    {
        &PAR_ICMP4_CODE,
        &PAR_ICMP4_PAYLOAD,
        &PAR_ICMP4_CHKSUM,
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};
static const Protocol PR_ICMP4_ECHO = {
    "icmp-echo",
    "ICMPv4 Echo Request (Ping)",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
    },
    {
        &PAR_ICMP4_ID,
        &PAR_ICMP4_SEQ,
        &PAR_ICMP4_PAYLOAD,
        &PAR_ICMP4_CHKSUM,
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};
static const Protocol PR_ICMP4_ECHOR = {
    "icmp-echo-reply",
    "ICMPv4 Echo Reply",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
    },
    {
        &PAR_ICMP4_ID,
        &PAR_ICMP4_SEQ,
        &PAR_ICMP4_PAYLOAD,
        &PAR_ICMP4_CHKSUM,
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};


static const Parameter PAR_TCP_SPORT = {
    "sport",
    "Source TCP port",
    Integer
};
static const Parameter PAR_TCP_DPORT = {
    "dport",
    "Destination TCP port",
    Integer
};
static const Parameter PAR_TCP_SEQ = {
    "seq",
    "TCP sequence number",
    Integer
};
static const Parameter PAR_TCP_ACK = {
    "ack",
    "TCP acknowledgment number",
    Integer
};
static const Parameter PAR_TCP_WIN = {
    "win",
    "TCP window size",
    Integer
};
static const Parameter PAR_TCP_URGPTR = {
    "urgptr",
    "TCP urgent pointer",
    Integer
};
static const Parameter PAR_TCP_FIN = {
    "FIN",
    "TCP FIN flag",
    Bit
};
static const Parameter PAR_TCP_SYN = {
    "SYN",
    "TCP SYN flag",
    Bit
};
static const Parameter PAR_TCP_RESET = {
    "RESET",
    "TCP RESET flag",
    Bit
};
static const Parameter PAR_TCP_PUSH = {
    "PUSH",
    "TCP PUSH flag",
    Bit
};
static const Parameter PAR_TCP_ACKFLAG = {
    "ACK",
    "TCP ACK flag",
    Bit
};
static const Parameter PAR_TCP_URGENT = {
    "URGENT",
    "TCP URGENT flag",
    Bit
};
static const Parameter PAR_TCP_ECN = {
    "ECN",
    "TCP ECN flag",
    Bit
};
static const Parameter PAR_TCP_CWR = {
    "CWR",
    "TCP CWR flag",
    Bit
};
static const Parameter PAR_TCP_NONCE = {
    "NONCE",
    "TCP nonce",
    Bit
};
static const Parameter PAR_TCP_PAYLOAD = {
    "payload",
    "TCP packet payload",
    Bytestream
};
static const Parameter PAR_TCP_CHKSUM = {
    "chksum",
    "TCP checksum",
    Integer
};
static const Protocol PR_TCP4 = {
    "tcp",
    "Raw TCP packet",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
        &PAR_TCP_SPORT,
        &PAR_TCP_DPORT,
        &PAR_TCP_SEQ,
        &PAR_TCP_ACK
    },
    {
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
        PAR_VLAN
    }
};


static const Parameter PAR_VXLAN_VNI = {
    "vni",
    "VXLAN Network Identifier",
    Integer
};
static const Parameter PAR_VXLAN_PAYLOAD = {
    "payload",
    "VXLAN payload data",
    Bytestream
};
static const Protocol PR_VXLAN4 = {
    "vxlan",
    "IPv4 Virtual eXtensible Local Area Network",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
        &PAR_UDP_SPORT,
        &PAR_UDP_DPORT,
        &PAR_VXLAN_VNI,
        &PAR_VXLAN_PAYLOAD,
    },
    {
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        PAR_VLAN,
    }
};
static const Protocol PR_VXLAN6 = {
    "vxlan6",
    "IPv6 Virtual eXtensible Local Area Network",
    {
        &PAR_ETH_DMAC,
        PAR_IP6,
        &PAR_UDP_SPORT,
        &PAR_UDP_DPORT,
        &PAR_VXLAN_VNI,
        &PAR_VXLAN_PAYLOAD,
    },
    {
        &PAR_ETH_SMAC,
        PAR_IP6_OPT,
        PAR_VLAN,
    }
};


static const Parameter PAR_GRE_PROTOCOL = PAR_IP_PROTOCOL;
static const Parameter PAR_GRE_KEY = {
    "key",
    "GRE key",
    Integer
};
static const Parameter PAR_GRE_SEQ = {
    "seq",
    "GRE sequence number",
    Integer
};
static const Parameter PAR_GRE_CHKSUM = {
    "chksum",
    "GRE checksum",
    Integer
};
static const Parameter PAR_GRE_PAYLOAD = {
    "payload",
    "GRE payload data",
    Bytestream
};
static const Protocol PR_GRE4 = {
    "gre",
    "IPv4 Generic Routing Encapsulation",
    {
        &PAR_ETH_DMAC,
        PAR_IP4,
    },
    {
        &PAR_ETH_SMAC,
        PAR_IP4_OPT,
        &PAR_GRE_KEY,
        &PAR_GRE_SEQ,
        &PAR_GRE_CHKSUM,
        &PAR_GRE_PAYLOAD,
        PAR_VLAN,
    }
};
static const Protocol PR_GRE6 = {
    "gre6",
    "IPv6 Generic Routing Encapsulation",
    {
        &PAR_ETH_DMAC,
        PAR_IP6,
    },
    {
        &PAR_ETH_SMAC,
        PAR_IP6_OPT,
        &PAR_GRE_KEY,
        &PAR_GRE_SEQ,
        &PAR_GRE_CHKSUM,
        &PAR_GRE_PAYLOAD,
        PAR_VLAN,
    }
};


static const std::vector<const Protocol*> all_protos
{
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
    &PR_VXLAN4,
    &PR_VXLAN6,
    &PR_GRE4,
    &PR_GRE6,
};

#endif
