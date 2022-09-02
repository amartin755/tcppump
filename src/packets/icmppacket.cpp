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


#include <cstring>

#include "bug.hpp"
#include "inetchecksum.hpp"
#include "icmppacket.hpp"



cIcmpPacket::cIcmpPacket ()
{
}

void cIcmpPacket::compileRaw (uint8_t type, uint8_t code, const uint8_t* payload, size_t len)
{
    BUG_ON (!payload);
    BUG_ON (!len);

    icmp_header_t header;

    header.type = type;
    header.code = code;
    header.checksum = 0;

    header.checksum = cInetChecksum::rfc1071((const uint16_t*)&header, sizeof(header), payload, len);
    cIPv4Packet::compile (PROTO_ICMP, (const uint8_t*)&header, sizeof (header), payload, len);
}

void cIcmpPacket::compileRaw (uint8_t type, uint8_t code, uint16_t checksum, const uint8_t* payload, size_t len)
{
    this->compileRaw (type, code, payload, len);

    icmp_header_t header;
    header.type = type;
    header.code = code;
    header.checksum = checksum;

    cIPv4Packet::updateL4Header((const uint8_t*)&header, sizeof (header));
}

void cIcmpPacket::compileWithEmbeddedInet (uint8_t type, uint8_t code, const uint8_t* inetheader, size_t len)
{
    icmp_empty_header_t header;
    header.head.type     = type;
    header.head.code     = code;
    header.unused        = 0;
    header.head.checksum = 0;
    if (!inetheader)
    {
        inetheader = compileGenericInetHeader (len);
    }
    header.head.checksum = cInetChecksum::rfc1071((const uint16_t*)&header, sizeof(header), inetheader, len);
    cIPv4Packet::compile (PROTO_ICMP, (const uint8_t*)&header, sizeof (header), inetheader, len);
}

void cIcmpPacket::compileRedirect (uint8_t code, const cIPv4& gw, const uint8_t* inetheader, size_t len)
{
    icmp_empty_header_t header;
    header.head.type     = 5;
    header.head.code     = code;
    std::memcpy (&header.unused, gw.getAsArray(), sizeof(header.unused));
    header.head.checksum = 0;
    if (!inetheader)
    {
        inetheader = compileGenericInetHeader (len);
    }
    header.head.checksum = cInetChecksum::rfc1071((const uint16_t*)&header, sizeof(header), inetheader, len);
    cIPv4Packet::compile (PROTO_ICMP, (const uint8_t*)&header, sizeof (header), inetheader, len);
}

void cIcmpPacket::compilePing (bool reply, uint16_t id, uint16_t seq, const uint8_t* data, size_t len)
{
    icmp_ping_t header;
    header.head.type = reply ? 0 : 8;
    header.head.code = 0;
    header.id        = htons(id);
    header.seq       = htons(seq);
    header.head.checksum = 0;
    header.head.checksum = cInetChecksum::rfc1071((const uint16_t*)&header, sizeof(header), data, len);
    cIPv4Packet::compile (PROTO_ICMP, (const uint8_t*)&header, sizeof (header), data, len);
}

const uint8_t* cIcmpPacket::compileGenericInetHeader (size_t& len)
{
    const ipv4_header_t& ipHeader = cIPv4Packet::getHeader();

    std::memset (&genInetHeader, 0, sizeof (genInetHeader));
    genInetHeader.ip.init ();

    genInetHeader.ip.srcIp = ipHeader.dstIp;
    genInetHeader.ip.dstIp = ipHeader.srcIp;
    genInetHeader.ip.protocol = PROTO_UDP;
    genInetHeader.ip.ttl = 64;
    genInetHeader.ip.totalLength = htons(genInetHeader.ip.getHeaderLenght() * 4 + sizeof (udp_header_t));
    genInetHeader.ip.chksum = cInetChecksum::rfc1071((const uint16_t*)&genInetHeader.ip, sizeof(genInetHeader.ip));

    genInetHeader.udp.srcPort = htons(42);
    genInetHeader.udp.dstPort = htons(4242);
    genInetHeader.udp.length  = htons (sizeof (udp_header_t));
    len = sizeof(genInetHeader);
    return (uint8_t*)&genInetHeader;
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cIcmpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
