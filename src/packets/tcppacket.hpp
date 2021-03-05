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


#ifndef TCP_PACKET_H_
#define TCP_PACKET_H_

#include <cstdint>

#include "ipv4packet.hpp"

#pragma pack(1)
typedef struct
{
    uint16_t srcPort;
    uint16_t dstPort;
    uint32_t seqNumber;
    uint32_t ackNumber;
    uint8_t  dataOffset;
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgentPtr;

    //TODO options

    void init (void)
    {
        std::memset (this, 0, sizeof (*this));
        setHeaderLenght (5);
        window = 1024;
    }
    void setHeaderLenght (int length)
    {
        dataOffset = uint8_t(length << 4);
    }

}tcp_header_t;
#pragma pack()


class cTcpPacket : public cIPv4Packet
{
public:
    cTcpPacket ();

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

    tcp_header_t header;
};


#endif /* TCP_PACKET_H_ */
