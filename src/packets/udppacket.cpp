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


#include <cstring>

#include "bugon.h"
#include "inet.h"
#include "udppacket.hpp"



cUdpPacket::cUdpPacket ()
{
    std::memset (&header, 0, sizeof(header));
}

void cUdpPacket::setPayload (const uint8_t* payload, size_t len)
{
    // TODO check max udp length

    header.length = htons(uint16_t(sizeof (header) + len));
    cIPv4Packet::setPayload (PROTO_UDP, (const uint8_t*)&header, sizeof (header), payload, len);
    header.checksum = calcChecksum();
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

// FIXME use cInetChecksum instead
uint16_t cUdpPacket::calcChecksum () const
{
    ipv4_pseudo_header_t ipPseudoHeader;
    const ipv4_header_t& ipHeader = cIPv4Packet::getHeader();

    ipPseudoHeader.srcIp    = ipHeader.srcIp;
    ipPseudoHeader.dstIp    = ipHeader.dstIp;
    ipPseudoHeader.nix      = 0;
    ipPseudoHeader.protocol = ipHeader.protocol;
    ipPseudoHeader.len      = htons((uint16_t)cIPv4Packet::getPayloadLength());

    uint32_t sum;

    sum  = csum ((const uint16_t*)&ipPseudoHeader, sizeof (ipPseudoHeader));
    sum += csum ();

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    uint16_t ret = (uint16_t)~sum;
    return ret ? ret : 0xffff;
}

uint32_t cUdpPacket::csum (const uint16_t* p, unsigned len) const
{
    uint32_t sum = 0;

    while (len > 1)
    {
        sum += *p++;
        len -= 2;
    }
    if (len)
        sum += *(uint8_t*)p;

    return sum;
}

uint32_t cUdpPacket::csum () const
{
    uint32_t sum = 0;
    unsigned offset = 0;
    unsigned len = (unsigned)cIPv4Packet::getPayloadLength ();

    while (len > 1)
    {
        sum += cIPv4Packet::getPayloadAt16(offset);
        len -= 2;
        offset += 2;
    }
    if (len)
        sum += cIPv4Packet::getPayloadAt8(offset);

    return sum;
}

#ifdef WITH_UNITTESTS
#include "console.hpp"

void cUdpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
