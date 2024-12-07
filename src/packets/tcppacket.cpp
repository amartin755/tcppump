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
#include "tcppacket.hpp"
#include "bug.hpp"


uint32_t cTcpPacket::sequence = 42;

cTcpPacket::cTcpPacket ()
{
    header.init();
    setSeqNumber (sequence);
}

void cTcpPacket::compile (const uint8_t* payload, size_t len, bool calcChksum)
{
    // TODO check max tcp length

    cIPv4Packet::compile (PROTO_TCP, (const uint8_t*)&header, sizeof (header), payload, len);
    if (calcChksum)
        header.checksum = calcChecksum();
    cIPv4Packet::updateL4Header ((const uint8_t*)&header, sizeof (header));
    sequence += (uint32_t)len + (uint32_t)header.isSyn();
}

void cTcpPacket::setSourcePort (uint16_t port)
{
    header.srcPort = htons(port);
}

void cTcpPacket::setDestinationPort (uint16_t port)
{
    header.dstPort = htons(port);
}

void cTcpPacket::setChecksum (uint16_t checksum)
{
    header.checksum = htons(checksum);
}

void cTcpPacket::setSeqNumber (uint32_t seq)
{
    header.seqNumber = htonl(seq);
    sequence = seq;
}

void cTcpPacket::setAckNumber (uint32_t ack)
{
    header.ackNumber = htonl(ack);
}

void cTcpPacket::setWindow (uint16_t window)
{
    header.window = htons(window);
}

void cTcpPacket::setUrgentPointer (uint16_t urgentPtr)
{
    header.urgentPtr = htons(urgentPtr);
}

void cTcpPacket::setFlagFIN (bool flag)
{
    header.setFin (flag);
}

void cTcpPacket::setFlagSYN (bool flag)
{
    header.setSyn (flag);
}

void cTcpPacket::setFlagRST (bool flag)
{
    header.setRst (flag);
}

void cTcpPacket::setFlagPSH (bool flag)
{
    header.setPsh (flag);
}

void cTcpPacket::setFlagACK (bool flag)
{
    header.setAck (flag);
}

void cTcpPacket::setFlagURG (bool flag)
{
    header.setUrg (flag);
}

void cTcpPacket::setFlagECE (bool flag)
{
    header.setEce (flag);
}

void cTcpPacket::setFlagCWR (bool flag)
{
    header.setCwr (flag);
}

void cTcpPacket::setFlagNON (bool flag)
{
    header.setNonce (flag);
}


// FIXME use cInetChecksum instead
uint16_t cTcpPacket::calcChecksum () const
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

uint32_t cTcpPacket::csum (const uint16_t* p, unsigned len) const
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

uint32_t cTcpPacket::csum () const
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

void cTcpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
