/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2019 Andreas Martin (netnag@mailbox.org)
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

#include "ethernetpacket.hpp"

#pragma pack(1)
typedef struct
{
    uint8_t  vers_ihl; // version (bit 0 - 3) | ip header length (bit 4 - 7)
    uint8_t  tos;
    uint16_t len;
    uint16_t ident;
    uint16_t flags_offset; // flags (bit 0 -2) | offset (bit 3 - 12)
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t chksum;
    ipv4_t   srcIp;
    ipv4_t   dstIp;

    void setVersion (int version)
    {
        vers_ihl = ((version & 0x0F) << 4) | (vers_ihl & 0x0F);
    }
    void setHeaderLenght (int length)
    {
        vers_ihl = (length & 0x0F) | (vers_ihl & 0xF0);
    }
    int getHeaderLenght ()
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
    void setFlags (bool reserved, bool df, bool mf)
    {
    	uint16_t fo = ntohs (flags_offset) & 0x1fff;
        flags_offset = htons((reserved ? 0x8000 : 0) | (df ? 0x4000 : 0) | (mf ? 0x2000 : 0) | fo);
    }
    void setOffset (int offset)
    {
        flags_offset = (offset & 0x1fff) | (flags_offset & 0xe000);
    }

}ipv4_header_t;
#pragma pack()


class cIPv4Packet
{
public:
    cIPv4Packet (cEthernetPacket&);
    void setDSCP (int dscp);
    void setECN (int ecn);
    void setTimeToLive (uint8_t ttl);
    void setDontFragment (bool df);
    void setSource (ipv4_t ip);
    void setDestination (ipv4_t ip);
    void setPayload (uint8_t protocol, const char* payload, size_t len);
    void updateHeaderChecksum ();

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    static uint16_t calcHeaderChecksum (const uint16_t* ipheader, int headerLen);

    cEthernetPacket& packet;
    ipv4_header_t    header;
};


#endif /* IPV4_PACKET_H_ */
