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
#include "udppacket.hpp"



cIcmpPacket::cIcmpPacket ()
{
}

void cIcmpPacket::compileRaw (uint8_t type, uint8_t code, const uint8_t* payload, size_t len)
{
    icmp_header_t header;

    header.type = type;
    header.code = code;
    header.checksum = 0;

    if (!payload && hasEmbeddedInetHeader(type))
    {
        const ipv4_header_t& ipHeader = cIPv4Packet::getHeader();
        struct
        {
            uint32_t null;
            ipv4_header_t ip;
            udp_header_t udp;
        }icmpPayload;

        std::memset (&icmpPayload, 0, sizeof (icmpPayload));
        icmpPayload.ip.init ();

        icmpPayload.ip.srcIp = ipHeader.dstIp;
        icmpPayload.ip.dstIp = ipHeader.srcIp;
        icmpPayload.ip.protocol = PROTO_UDP;
        icmpPayload.ip.ttl = 64;
        icmpPayload.ip.totalLength = htons(icmpPayload.ip.getHeaderLenght() * 4 + sizeof (udp_header_t));
        icmpPayload.ip.chksum = cInetChecksum::rfc1071((const uint16_t*)&icmpPayload.ip, sizeof(icmpPayload.ip));

        icmpPayload.udp.length = htons (sizeof (udp_header_t));
        payload = (uint8_t*)&icmpPayload;
        len = sizeof(icmpPayload);
    }

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

bool cIcmpPacket::hasEmbeddedInetHeader (unsigned type)
{
    switch (type)
    {
    case 3:
    case 4:
    case 5:
    case 11:
    case 12:
        return true;
    }
    return false;
}

#ifdef WITH_UNITTESTS
#include "console.hpp"

void cIcmpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
