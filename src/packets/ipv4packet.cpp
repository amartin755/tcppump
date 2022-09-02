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
#include "ipv4packet.hpp"

#include "bug.hpp"
#include "inetchecksum.hpp"
#include "settings.hpp"


uint16_t cIPv4Packet::identification = 1;


cIPv4Packet::cIPv4Packet ()
{
    ipHeader.init();

    mtu = cSettings::get().getMyMTU();

    cEthernetPacket firstPacket;
    firstPacket.setTypeLength (ETHERTYPE_IPV4);
    packets.push_back(std::move(firstPacket));
    packetsAsArray = nullptr;
    hasId = false;
}

cIPv4Packet::~cIPv4Packet ()
{
    delete[] packetsAsArray;
    packetsAsArray = nullptr;
}

cEthernetPacket& cIPv4Packet::getFirstEthernetPacket ()
{
    BUG_ON (packets.size() <= 0);
    return packets.front();
}

std::list<cEthernetPacket>& cIPv4Packet::getAllEthernetPackets (void)
{
    return packets;
}

void cIPv4Packet::setDestMac (const cMacAddress& dest)
{
    for (auto & p : packets)
    {
        p.setDestMac(dest);
    }
}

void cIPv4Packet::setDSCP (int dscp)
{
    ipHeader.ip.setDSCP (dscp);
}

void cIPv4Packet::setECN (int ecn)
{
    ipHeader.ip.setECN (ecn);
}

void cIPv4Packet::setTimeToLive (uint8_t ttl)
{
    ipHeader.ip.ttl = ttl;
}

void cIPv4Packet::setDontFragment (bool df)
{
    ipHeader.ip.setFlagDF (df);
}

void cIPv4Packet::setSource (const cIPv4& ip)
{
    ipHeader.ip.srcIp = ip.get();
}

void cIPv4Packet::getSource (cIPv4& ip) const
{
    ip.set(ipHeader.ip.srcIp);
}

void cIPv4Packet::setDestination (const cIPv4& ip)
{
    ipHeader.ip.dstIp = ip.get();
}

void cIPv4Packet::getDestination (cIPv4& ip) const
{
    ip.set(ipHeader.ip.dstIp);
}

void cIPv4Packet::setIdentification (uint16_t id)
{
    ipHeader.ip.ident = htons(id);
    hasId = true;
}

void cIPv4Packet::compile (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen)
{
    if (l4headerLen + payloadLen + getHeaderLength() > 65535)
        throw FormatException (exParRange, nullptr);

    size_t ipHeaderLen = getHeaderLength();

    unsigned fragCnt = unsigned((l4headerLen + payloadLen - 1) / (mtu - ipHeaderLen)) + 1;
    size_t offset = 0;

    BUG_ON (l4headerLen > mtu - ipHeaderLen);    // we rely on L4 header fitting into first ip fragment

    cEthernetPacket &packet = packets.front();

    // if there is no destination mac AND we have an ip multicast, translate to mac multicast
    if (!packet.hasDestMac())
    {
        cIPv4 dstIp (ipHeader.ip.dstIp);
        if (dstIp.isMulticast ())
        {
            packet.setDestMac (cMacAddress (1, 0, 0x5e, dstIp.getAsArray()[1] & 0x7f, dstIp.getAsArray()[2], dstIp.getAsArray()[3]));
        }
    }

    ipHeader.ip.protocol = protocol;
    if (fragCnt > 1 && !hasId)
        ipHeader.ip.ident = htons(identification++);

    for (unsigned n = 1; n < fragCnt; n++)
        packets.push_back(std::move(cEthernetPacket(packet)));

    packetsAsArray = new const cEthernetPacket*[fragCnt];

    unsigned n = 0;
    for (auto & p : packets)
    {
        packetsAsArray[n] = &p;

        size_t fragLen = 0;
        if (n == 0)
            fragLen = l4headerLen + payloadLen + ipHeaderLen > mtu ? mtu - ipHeaderLen : l4headerLen + payloadLen;
        else
            fragLen = payloadLen + ipHeaderLen > mtu ? mtu - ipHeaderLen : payloadLen;

        if ((n + 1) != packets.size())
        {
            fragLen /= 8;
            fragLen *= 8;
        }
        ipHeader.ip.totalLength = htons (uint16_t(ipHeaderLen + fragLen));
        ipHeader.ip.setFlagMF (n + 1 != fragCnt);
        ipHeader.ip.setOffset ((unsigned)offset);
        updateHeaderChecksum ();

        p.setPayload ((uint8_t*)&ipHeader, ipHeaderLen);           // write IP header

        if (n == 0)
        {
            fragLen -= l4headerLen;
            if (l4headerLen && l4header)
                p.appendPayload(l4header, l4headerLen);            // copy L4 header (e.g. udp header)
            if (payloadLen && payload)
                p.appendPayload (payload, fragLen);  // copy payload

            payloadLen -= fragLen;
            offset  += fragLen + l4headerLen;
        }
        else
        {
            p.appendPayload (payload, fragLen);                    // copy payload
            payloadLen -= fragLen;
            offset  += fragLen;
        }

        payload += fragLen;
        n++;
    }
}

uint8_t cIPv4Packet::getPayloadAt8 (unsigned offset) const
{
    size_t ipHeaderLen  = getHeaderLength();
    unsigned fragment   = static_cast<unsigned>(offset / (mtu - ipHeaderLen));
    unsigned packOffset = static_cast<unsigned>(offset % (mtu - ipHeaderLen));

    const cEthernetPacket *packet = packetsAsArray[fragment];
    return packet->getPayloadAt8 (packOffset + (unsigned)ipHeaderLen);
}

// note: offset is a byte offset!!!
uint16_t cIPv4Packet::getPayloadAt16 (unsigned offset) const
{
    size_t ipHeaderLen  = getHeaderLength();
    unsigned fragment   = static_cast<unsigned>(offset / (mtu - ipHeaderLen));
    unsigned packOffset = static_cast<unsigned>(offset % (mtu - ipHeaderLen));

    const cEthernetPacket *packet = packetsAsArray[fragment];
    return packet->getPayloadAt16 (packOffset + (unsigned)ipHeaderLen);
}

size_t cIPv4Packet::getPayloadLength () const
{
    size_t ipHeaderLen = getHeaderLength();
    size_t len = 0;

    for (auto & p : packets)
    {
        BUG_ON (p.getPayloadLength () <= ipHeaderLen);
        len += p.getPayloadLength () - ipHeaderLen;
    }

    return len;
}

void cIPv4Packet::updateL4Header (const uint8_t* l4header, size_t l4headerLen)
{
    cEthernetPacket &packet = packets.front();

    BUG_ON (packet.getPayloadLength () <= getHeaderLength());

    packet.updatePayloadAt((unsigned)getHeaderLength(), l4header, l4headerLen);
}

void cIPv4Packet::updateHeaderChecksum ()
{
    ipHeader.ip.chksum = 0;
    ipHeader.ip.chksum = cInetChecksum::rfc1071((const uint16_t*)&ipHeader, getHeaderLength(), nullptr, 0);
}

void cIPv4Packet::addRouterAlertOption (void)
{
    ipHeader.ip.setHeaderLenght(ipHeader.ip.getHeaderLenght() + sizeof (ipv4_option_router_alert_t) / 4);
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cIPv4Packet::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    // FIXME unit tests!
}
#endif
