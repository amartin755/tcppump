// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2021 Andreas Martin (netnag@mailbox.org)
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
        window = htons(1024);
    }
    void setHeaderLenght (int length)
    {
        dataOffset = uint8_t(length << 4);
    }
    void setFin (bool f)
    {
        flags = f ? flags | 1 : flags & ~1;
    }
    void setSyn (bool f)
    {
        flags = f ? flags | 2 : flags & ~2;
    }
    void setRst (bool f)
    {
        flags = f ? flags | 4 : flags & ~4;
    }
    void setPsh (bool f)
    {
        flags = f ? flags | 8 : flags & ~8;
    }
    void setAck (bool f)
    {
        flags = f ? flags | 0x10 : flags & ~0x10;
    }
    void setUrg (bool f)
    {
        flags = f ? flags | 0x20 : flags & ~0x20;
    }
    void setEce (bool f)
    {
        flags = f ? flags | 0x40 : flags & ~0x40;
    }
    void setCwr (bool f)
    {
        flags = f ? flags | 0x80 : flags & ~0x80;
    }
    void setNonce (bool f)
    {
        dataOffset = f ? dataOffset | 1 : dataOffset & ~1;
    }
    bool isSyn (void)
    {
        return !!(flags & 2);
    }

}tcp_header_t;
#pragma pack()


class cTcpPacket : public cIPv4Packet
{
public:
    cTcpPacket ();

    void compile (const uint8_t* payload, size_t len, bool calcChecksum);
    void setSourcePort (uint16_t port);
    void setDestinationPort (uint16_t port);
    void setChecksum (uint16_t checksum);
    void setSeqNumber (uint32_t seq);
    void setAckNumber (uint32_t ack);
    void setWindow (uint16_t window);
    void setUrgentPointer (uint16_t urgentPtr);
    void setFlagFIN (bool flag);
    void setFlagSYN (bool flag);
    void setFlagRST (bool flag);
    void setFlagPSH (bool flag);
    void setFlagACK (bool flag);
    void setFlagURG (bool flag);
    void setFlagECE (bool flag);
    void setFlagCWR (bool flag);
    void setFlagNON (bool flag);


#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    uint16_t calcChecksum () const;
    uint32_t csum (const uint16_t* p, unsigned len) const;
    uint32_t csum () const;

    tcp_header_t header;
    static uint32_t sequence;
};


#endif /* TCP_PACKET_H_ */
