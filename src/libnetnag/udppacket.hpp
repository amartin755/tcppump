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


#ifndef UDP_PACKET_H_
#define UDP_PACKET_H_

#include <cstdint>

#include "ipv4packet.hpp"

#pragma pack(1)
typedef struct
{
    uint16_t srcPort;
    uint16_t dstPort;
    uint16_t length;
    uint16_t checksum;

}udp_header_t;
#pragma pack()


class cUdpPacket : public cIPv4Packet
{
public:
    cUdpPacket ();

    void setPayload (const uint8_t* payload, size_t len);
    void setSourcePort (uint16_t port);
    void setDestinationPort (uint16_t port);
    void setChecksum (uint16_t checksum);


#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    uint16_t calcChecksum () const;
    uint32_t csum (const uint16_t* p, unsigned len) const;
    uint32_t csum () const;

    udp_header_t header;
};


#endif /* UDP_PACKET_H_ */
