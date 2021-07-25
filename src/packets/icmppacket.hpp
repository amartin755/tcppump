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


#ifndef ICMP_PACKET_H_
#define ICMP_PACKET_H_

#include <cstdint>

#include "ipv4packet.hpp"
#include "udppacket.hpp"

#pragma pack(1)
typedef struct
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;

}icmp_header_t;

typedef struct
{
    icmp_header_t head;
    uint32_t      unused;
}icmp_empty_header_t;

typedef struct
{
    ipv4_header_t ip;
    udp_header_t udp;
}generic_inet_header;

#pragma pack()


class cIcmpPacket : public cIPv4Packet
{
public:
    cIcmpPacket ();

    void compileRaw (uint8_t type, uint8_t code, uint16_t checksum, const uint8_t* payload, size_t len);
    void compileRaw (uint8_t type, uint8_t code, const uint8_t* payload, size_t len);
    void compileWithEmbeddedInet (uint8_t type, uint8_t code, const uint8_t* inetheader, size_t len);
    void compileRedirect (uint8_t code, const cIpAddress& gw, const uint8_t* inetheader, size_t len);


#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    const uint8_t* compileGenericInetHeader (size_t& len);
    generic_inet_header genInetHeader;
};


#endif /* ICMP_PACKET_H_ */
