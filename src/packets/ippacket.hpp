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


#ifndef IP_PACKET_H_
#define IP_PACKET_H_

#include <cstdint>
#include <list>

#include "inet.h" // ntohs, htons
#include "ethernetpacket.hpp"
#include "ipaddress.hpp"
#include "linkable.hpp"
#include "inetchecksum.hpp"


// RFC2113
struct ipv4_option_router_alert_t
{
    uint8_t  type;   // 0x94
    uint8_t  length; // 4
    uint16_t value;  // 0 -> Router shall examine packet

};
static_assert (sizeof (ipv4_option_router_alert_t) == 4, "ipv4_option_router_alert_t is not packed");

struct ipv4_header_t
{
    uint8_t  vers_ihl; // version (bit 0 - 3) | ip header length (bit 4 - 7)
    uint8_t  tos;
    uint16_t totalLength;
    uint16_t ident;
    uint16_t flags_offset; // flags (bit 0 -2) | offset (bit 3 - 12)
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t chksum;
    in_addr  srcIp;
    in_addr  dstIp;

    void init (void)
    {
        std::memset (this, 0, sizeof (*this));
        setVersion (4);
        setHeaderLenght (5);
    }

    void setVersion (int version)
    {
        vers_ihl = ((version & 0x0F) << 4) | (vers_ihl & 0x0F);
    }
    void setHeaderLenght (unsigned length)
    {
        vers_ihl = (length & 0x0F) | (vers_ihl & 0xF0);
    }
    unsigned getHeaderLenght () const
    {
        return (unsigned)(vers_ihl & 0x0F);
    }
    void setDSCP (int dscp)
    {
        tos = ((dscp & 0x3F) << 2) | (tos & 0x03);
    }
    void setECN (int ecn)
    {
        tos = (ecn & 0x03) | (tos & 0xfc);
    }
    void setFlagDF (bool df)
    {
        uint16_t fo  = ntohs (flags_offset) & 0xbfff;
        flags_offset = htons ((df ? 0x4000 : 0) | fo);
    }
    void setFlagMF (bool mf)
    {
        uint16_t fo  = ntohs (flags_offset) & 0xdfff;
        flags_offset = htons ((mf ? 0x2000 : 0) | fo);
    }
    void setOffset (unsigned offset)
    {
        BUG_ON (offset % 8);
        offset /= 8;
        uint16_t fo  = ntohs (flags_offset) & 0xe000;
        flags_offset = htons (uint16_t(offset & ~0xe000) | fo);
    }

};
static_assert (sizeof (ipv4_header_t) == 20, "ipv4_header_t is not packed");

struct ipv4_header_with_router_alert_t : public ipv4_header_t
{
    ipv4_option_router_alert_t routerAlert;

    void compile (const in_addr &srcIp, const in_addr &dstIp, unsigned ttl,
        uint8_t proto, unsigned dscp, unsigned ecn, bool df, bool mf, unsigned offset, unsigned hdrLen, uint16_t totalLen, uint16_t ident, bool withRouterAlert)
    {
        BUG_ON (offset % 8);

        vers_ihl          = 4 << 4 | ((hdrLen / 4) & 0x0f);
        tos               = ((dscp & 0x3F) << 2) | (ecn & 0x03);
        flags_offset      = htons ((df ? 0x4000 : 0) | (mf ? 0x2000 : 0) | ((offset / 8) & 0x1fff));

        this->srcIp       = srcIp;
        this->dstIp       = dstIp;
        this->ttl         = ttl;
        this->protocol    = proto;
        this->totalLength = htons (totalLen);
        this->ident       = htons (ident);

        if (withRouterAlert)
        {
            routerAlert.type   = 0x94;
            routerAlert.length = 4;
            routerAlert.value  = 0;
        }
        this->chksum = 0;
        this->chksum = cInetChecksum::rfc1071 ((const void*)this, hdrLen);
    }
};
static_assert (sizeof (ipv4_header_with_router_alert_t) == 24, "ipv4_header_with_router_alert_t is not packed");

struct ipv4_pseudo_header_t
{
    struct in_addr srcIp;
    struct in_addr dstIp;
    uint8_t        nix;         // 0
    uint8_t        protocol;
    uint16_t       len;
};
static_assert (sizeof (ipv4_pseudo_header_t) == 12, "ipv4_pseudo_header_t is not packed");

struct ipv6_header_t
{
    uint8_t  vers_tc; // version (bit 0 - 3) | traffic class (bit 4 - 7)
    uint8_t  tc_fl;
    uint16_t fl;
    uint16_t payloadLength;
    uint8_t  nextHeader;
    uint8_t  hopLimit;
    struct in6_addr srcIp;
    struct in6_addr dstIp;

    void init (void)
    {
        std::memset (this, 0, sizeof (*this));
        vers_tc = 6 << 4;
    }
    void setDSCP (unsigned dscp)
    {
        dscp &= 0x3f;
        vers_tc = (6 << 4) | (dscp >> 2);
        tc_fl = (dscp & 3) << 6 | (tc_fl & 0x3f);
    }
    void setECN (unsigned ecn)
    {
        tc_fl = (ecn & 3) << 4 | (tc_fl & 0xcf);
    }
    void setFlowLabel (unsigned fl)
    {
        tc_fl = (fl & 0x0F0000) >> 16 | (tc_fl & 0xf0);
    }
    void setPayloadLen (uint16_t len)
    {
        payloadLength = htons (len);
    }
    void setNextHeader (uint8_t nh)
    {
        nextHeader = nh;
    }
    void setHopLimit (uint8_t hl)
    {
        hopLimit = hl;
    }

    void compile (const in6_addr &srcIp, const in6_addr &dstIp, unsigned ttl,
        uint8_t proto, unsigned dscp, unsigned ecn, unsigned flowlabel, uint16_t paylodLen)
    {
        vers_tc = (6 << 4) | ((dscp & 0x3f) >> 2);
        tc_fl = ((dscp & 3) << 6) | ((ecn & 3) << 4) | ((flowlabel & 0x0f0000) >> 16);
        fl = htons(flowlabel & 0xffff);
        payloadLength = htons (paylodLen);
        nextHeader = proto;
        hopLimit = ttl;
        std::memcpy ((void*)&this->srcIp, &srcIp, sizeof (this->srcIp));
        std::memcpy ((void*)&this->dstIp, &dstIp, sizeof (this->dstIp));
    }
};
static_assert (sizeof (ipv6_header_t) == 40, "ipv6_header_t is not packed");


class cIPPacket : public cLinkable
{
public:
    cIPPacket (bool isIPv6 = false);
    virtual ~cIPPacket ();
    void setDSCP (unsigned dscp);
    void setECN (unsigned ecn);
    void setTimeToLive (uint8_t ttl);
    cEthernetPacket& getFirstEthernetPacket ();
    std::list<cEthernetPacket>& getAllEthernetPackets (void);
    void setDestMac (const cMacAddress& dest);
    void compile (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen)
    {
        if (m_isIPv6)
            v6compile (protocol, l4header, l4headerLen, payload, payloadLen);
        else
            v4compile (protocol, l4header, l4headerLen, payload, payloadLen);
    }

    // IPv4 specific
    void setDontFragment (bool df);
    void setIdentification (uint16_t id);
    void setSource (const cIPv4& ip);
    void setDestination (const cIPv4& ip);
    void getSource (cIPv4& ip) const;
    void getDestination (cIPv4& ip) const;

    // IPv6 specific
    void setSource (const cIPv6& ip);
    void setDestination (const cIPv6& ip);
    void getSource (cIPv6& ip) const;
    void getDestination (cIPv6& ip) const;

    enum protocols
    {
        PROTO_ICMP    = 1,
        PROTO_IGMP    = 2,
        PROTO_TCP     = 6,
        PROTO_UDP     = 17,
        PROTO_GRE     = 47,
        PROTO_ICMPv6  = 58,
        PROTO_VRRP    = 112,
    };

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

protected:
    size_t   getPayloadLength () const;
    void     updateL4Header (const uint8_t* l4header, size_t l4headerLen);
    void     addRouterAlertOption (void);

private:
    size_t getHeaderLength () const
    {
        if (m_isIPv6)
            return sizeof (ipv6_header_t);
        else
            return m_v4.hasRouterAlertOption ? sizeof (ipv4_header_with_router_alert_t) : sizeof (ipv4_header_t);
    }
    void v4compile (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen);
    void v6compile (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen);

    bool                        m_isIPv6;
    unsigned                    m_mtu;
    std::list<cEthernetPacket>  m_packets;
    const cEthernetPacket**     m_packetsAsArray;


    // IP header content
    //  generic
    unsigned m_dscp;
    unsigned m_ecn;
    uint8_t  m_ttl;

    //  IPv4 specific
    struct v4
    {
        struct in_addr  srcIP;
        struct in_addr  dstIP;
        bool            dontFragment;
        uint16_t        identification;
        static uint16_t nextIdentification;
        bool            hasId;
        bool            hasRouterAlertOption;
    } m_v4;

    //  IPv6 specific
    struct v6
    {
        struct in6_addr srcIP;
        struct in6_addr dstIP;
        unsigned        flowlabel;
    } m_v6;

};


#endif /* IP_PACKET_H_ */
