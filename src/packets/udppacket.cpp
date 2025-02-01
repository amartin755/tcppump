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

#include "inet.h"
#include "udppacket.hpp"
#include "bug.hpp"
#include "inetchecksum.hpp"


cUdpPacket::cUdpPacket (bool isIPv6) : cIPPacket (isIPv6)
{
    std::memset (&header, 0, sizeof(header));
}

void cUdpPacket::compile (const uint8_t* payload, size_t len)
{
    // TODO check max udp length

    header.length = htons(uint16_t(sizeof (header) + len));
    cIPPacket::compile (PROTO_UDP, (const uint8_t*)&header, sizeof (header), payload, len);
    header.checksum = calcChecksum(payload, len);
    cIPPacket::updateL4Header ((const uint8_t*)&header, sizeof (header));
}

void cUdpPacket::setSourcePort (uint16_t port)
{
    header.srcPort = htons(port);
}

void cUdpPacket::setDestinationPort (uint16_t port)
{
    header.dstPort = htons(port);
}

void cUdpPacket::setChecksum (uint16_t checksum)
{
    header.checksum = htons(checksum);
    cIPPacket::updateL4Header ((const uint8_t*)&header, sizeof (header));
}

uint16_t cUdpPacket::calcChecksum (const uint8_t* payload, size_t len) const
{
    uint16_t chksum;
    if (!isIPv6())
    {
        cIPv4 src, dst;
        getSource (src);
        getDestination (dst);
        const ipv4_pseudo_header_t ipPseudoHeader = {
            src.get(),
            dst.get(),
            0,
            PROTO_UDP,
            htons((uint16_t)cIPPacket::getPayloadLength())
        };

        chksum = cInetChecksum::rfc1071 ((const void*)&ipPseudoHeader, sizeof (ipPseudoHeader), &header, sizeof(header), payload, len);
    }
    else
    {
        cIPv6 src, dst;
        getSource (src);
        getDestination (dst);
        const ipv6_pseudo_header_t ipPseudoHeader = {
            src.get(),
            dst.get(),
            htons((uint32_t)cIPPacket::getPayloadLength()),
            0,
            0,
            0,
            PROTO_UDP,
        };

        chksum = cInetChecksum::rfc1071 ((const void*)&ipPseudoHeader, sizeof (ipPseudoHeader), &header, sizeof(header), payload, len);
    }
    return chksum ? chksum : 0xffff;
}

#ifdef WITH_UNITTESTS
#include "console.hpp"

void cUdpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
