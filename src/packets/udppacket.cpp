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


cUdpPacket::cUdpPacket ()
{
    std::memset (&header, 0, sizeof(header));
}

void cUdpPacket::compile (const uint8_t* payload, size_t len)
{
    // TODO check max udp length

    header.length = htons(uint16_t(sizeof (header) + len));
    cIPv4Packet::compile (PROTO_UDP, (const uint8_t*)&header, sizeof (header), payload, len);
    header.checksum = calcChecksum(payload, len);
    cIPv4Packet::updateL4Header ((const uint8_t*)&header, sizeof (header));
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
    cIPv4Packet::updateL4Header ((const uint8_t*)&header, sizeof (header));
}

uint16_t cUdpPacket::calcChecksum (const uint8_t* payload, size_t len) const
{
    ipv4_pseudo_header_t ipPseudoHeader;
    const ipv4_header_t& ipHeader = cIPv4Packet::getHeader();

    ipPseudoHeader.srcIp    = ipHeader.srcIp;
    ipPseudoHeader.dstIp    = ipHeader.dstIp;
    ipPseudoHeader.nix      = 0;
    ipPseudoHeader.protocol = ipHeader.protocol;
    ipPseudoHeader.len      = htons((uint16_t)cIPv4Packet::getPayloadLength());

    uint16_t ret = cInetChecksum::rfc1071 ((const uint16_t*)&ipPseudoHeader, sizeof (ipPseudoHeader), &header, sizeof(header), payload, len);
    return ret ? ret : 0xffff;
}

#ifdef WITH_UNITTESTS
#include "console.hpp"

void cUdpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
