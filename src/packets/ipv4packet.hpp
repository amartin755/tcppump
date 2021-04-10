/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2020 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef IPV4_PACKET_H_
#define IPV4_PACKET_H_

#include <cstdint>
#include <list>

#include "inet.h" // ntohs, htons
#include "ethernetpacket.hpp"
#include "ipaddress.hpp"
#include "linkable.hpp"

#pragma pack(1)
// RFC2113
typedef struct
{
    uint8_t  type;   // 0x94
    uint8_t  length; // 4
    uint16_t value;  // 0 -> Router shall examine packet

}ipv4_option_router_alert_t;

typedef struct
{
    uint8_t  vers_ihl; // version (bit 0 - 3) | ip header length (bit 4 - 7)
    uint8_t  tos;
    uint16_t totalLength;
    uint16_t ident;
    uint16_t flags_offset; // flags (bit 0 -2) | offset (bit 3 - 12)
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t chksum;
    struct in_addr srcIp;
    struct in_addr dstIp;

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
    void setHeaderLenght (int length)
    {
        vers_ihl = (length & 0x0F) | (vers_ihl & 0xF0);
    }
    int getHeaderLenght () const
    {
        return (int)(vers_ihl & 0x0F);
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
        flags_offset = htons ((offset & ~0xe000) | fo);
    }

}ipv4_header_t;

typedef struct
{
    ipv4_header_t ip;
    ipv4_option_router_alert_t routerAlert;

    void init (void)
    {
        ip.init();
        routerAlert.type   = 0x94;
        routerAlert.length = 4;
        routerAlert.value  = 0;
    }
}ipv4_header_with_router_alert_t;

typedef struct
{
    struct in_addr srcIp;
    struct in_addr dstIp;
    uint8_t        nix;         // 0
    uint8_t        protocol;
    uint16_t       len;
}ipv4_pseudo_header_t;

#pragma pack()


class cIPv4Packet : public cLinkable
{
public:
    cIPv4Packet ();
    virtual ~cIPv4Packet ();
    void setDSCP (int dscp);
    void setECN (int ecn);
    void setTimeToLive (uint8_t ttl);
    void setDontFragment (bool df);
    void setSource (const cIpAddress& ip);
    void setDestination (const cIpAddress& ip);
    void getSource (cIpAddress& ip) const;
    void getDestination (cIpAddress& ip) const;
    void setIdentification (uint16_t id);
    cEthernetPacket& getFirstEthernetPacket ();
    std::list<cEthernetPacket>& getAllEthernetPackets (void);
    void setDestMac (const cMacAddress& dest);
    void compile (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen);

    enum protocols
    {
        PROTO_ICMP = 1,
        PROTO_IGMP = 2,
        PROTO_TCP  = 6,
        PROTO_UDP  = 17,
        PROTO_VRRP = 112,
    };

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

protected:
    const ipv4_header_t& getHeader () const
    {
        return ipHeader.ip;
    }
    uint8_t  getPayloadAt8 (unsigned offset) const;
    uint16_t getPayloadAt16 (unsigned offset) const; // note: offset is a byte offset!!!
    size_t   getPayloadLength () const;
    void     updateL4Header (const uint8_t* l4header, size_t l4headerLen);
    void     addRouterAlertOption (void);
    size_t   getHeaderLength () const
    {
        return ipHeader.ip.getHeaderLenght() * 4;
    }
    void updateHeaderChecksum ();


private:
    ipv4_header_with_router_alert_t ipHeader;
    std::list<cEthernetPacket>      packets;
    unsigned                        mtu;
    const cEthernetPacket**         packetsAsArray;

    static uint16_t identification;
    bool hasId;

};


#endif /* IPV4_PACKET_H_ */
