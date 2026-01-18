// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
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
#include "inet.h"
#include "tcppacket.hpp"
#include "inetchecksum.hpp"


uint32_t cTcpPacket::sequence = 42;

cTcpPacket::cTcpPacket (bool isIPv6) : cIPPacket (isIPv6)
{
    header.init();
    setSeqNumber (sequence);
}

void cTcpPacket::compile (const uint8_t* payload, size_t len, bool calcChksum)
{
    // TODO check max tcp length

    cIPPacket::compile (PROTO_TCP, (const uint8_t*)&header, sizeof (header), payload, len);
    if (calcChksum)
        header.checksum = calcChecksum(payload, len);
    cIPPacket::updateL4Header ((const uint8_t*)&header, sizeof (header));
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


uint16_t cTcpPacket::calcChecksum (const uint8_t* payload, size_t len) const
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
            PROTO_TCP,
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
            htonl((uint32_t)cIPPacket::getPayloadLength()),
            {0, 0, 0},
            PROTO_TCP,
        };

        chksum = cInetChecksum::rfc1071 ((const void*)&ipPseudoHeader, sizeof (ipPseudoHeader), &header, sizeof(header), payload, len);
    }
    return chksum ? chksum : 0xffff;
}

#ifdef WITH_UNITTESTS
#include "console.hpp"

void cTcpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
