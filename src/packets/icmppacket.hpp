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

#include "ippacket.hpp"
#include "udppacket.hpp"


struct icmp_header_t
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;

};
static_assert (sizeof (icmp_header_t) == 4, "icmp_header_t is not packed");

struct icmp_empty_header_t
{
    icmp_header_t head;
    uint32_t      unused;
};
static_assert (sizeof (icmp_empty_header_t) == 8, "icmp_empty_header_t is not packed");

struct generic_inet_header
{
    ipv4_header_t ip;
    udp_header_t udp;
};
static_assert (sizeof (generic_inet_header) == 28, "generic_inet_header is not packed");

struct icmp_ping_t
{
    icmp_header_t head;
    uint16_t      id;
    uint16_t      seq;
};
static_assert (sizeof (icmp_ping_t) == 8, "icmp_ping_t is not packed");


class cIcmpPacket : public cIPPacket
{
public:
    cIcmpPacket ();

    void compileRaw (uint8_t type, uint8_t code, uint16_t checksum, const uint8_t* payload, size_t len);
    void compileRaw (uint8_t type, uint8_t code, const uint8_t* payload, size_t len);
    void compileWithEmbeddedInet (uint8_t type, uint8_t code, const uint8_t* inetheader, size_t len);
    void compileRedirect (uint8_t code, const cIPv4& gw, const uint8_t* inetheader, size_t len);
    void compilePing (bool reply, uint16_t id, uint16_t seq, const uint8_t* data, size_t len);


#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    const uint8_t* compileGenericInetHeader (size_t& len);
    generic_inet_header genInetHeader;
};


#endif /* ICMP_PACKET_H_ */
