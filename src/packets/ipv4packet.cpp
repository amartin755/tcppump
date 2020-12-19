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

#include "inet.h"
#include "ipv4packet.hpp"

#include "bug.hpp"
#include "inetchecksum.hpp"


uint16_t cIPv4Packet::identification = 1;


cIPv4Packet::cIPv4Packet ()
{
    ipHeader.init();

    mtu = 1500; // FIXME real MTU

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
    BUG_ON (packets.size() > 0);
    return packets.front();
}

size_t cIPv4Packet::getAllEthernetPackets (std::list<cEthernetPacket>& l) const
{
    size_t ret = packets.size();

    for (auto & p : packets)
    {
        l.push_back(std::move(p));
    }


    return ret;
}

void cIPv4Packet::setDSCP (int dscp)
{
    ipHeader.setDSCP (dscp);
}

void cIPv4Packet::setECN (int ecn)
{
    ipHeader.setECN (ecn);
}

void cIPv4Packet::setTimeToLive (uint8_t ttl)
{
    ipHeader.ttl = ttl;
}

void cIPv4Packet::setDontFragment (bool df)
{
    ipHeader.setFlagDF (df);
}

void cIPv4Packet::setSource (const cIpAddress& ip)
{
    ipHeader.srcIp = ip.get();
}

void cIPv4Packet::setDestination (const cIpAddress& ip)
{
    ipHeader.dstIp = ip.get();
}

void cIPv4Packet::setIdentification (uint16_t id)
{
    ipHeader.ident = htons(id);
    hasId = true;
}

void cIPv4Packet::compile (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen)
{
    if (l4headerLen + payloadLen + getHeaderLength() > 65535)
        throw FormatException (exParRange, nullptr);

    size_t ipHeaderLen = getHeaderLength();

    unsigned fragCnt = unsigned((l4headerLen + payloadLen - 1) / (mtu - ipHeaderLen)) + 1;
    unsigned offset = 0;

    BUG_ON (l4headerLen < mtu - ipHeaderLen);    // we rely on L4 header fitting into first ip fragment

    cEthernetPacket &packet = packets.front();

    // if there is no destination mac AND we have an ip multicast, translate to mac multicast
    if (!packet.hasDestMac())
    {
        cIpAddress dstIp (ipHeader.dstIp);
        if (dstIp.isMulticast ())
        {
            packet.setDestMac (cMacAddress (1, 0, 0x5e, dstIp.getAsArray()[1] & 0x7f, dstIp.getAsArray()[2], dstIp.getAsArray()[3]));
        }
    }

    ipHeader.protocol = protocol;
    if (fragCnt > 1 && !hasId)
        ipHeader.ident = htons(identification++);

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

        ipHeader.totalLength = htons (uint16_t(ipHeaderLen + fragLen));
        ipHeader.setFlagMF (n + 1 != fragCnt);
        ipHeader.setOffset (offset);
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
    unsigned fragment   = offset / (mtu - ipHeaderLen);
    unsigned packOffset = offset % (mtu - ipHeaderLen);

    const cEthernetPacket *packet = packetsAsArray[fragment];
    return packet->getPayloadAt8 (packOffset + (unsigned)ipHeaderLen);
}

// note: offset is a byte offset!!!
uint16_t cIPv4Packet::getPayloadAt16 (unsigned offset) const
{
    size_t ipHeaderLen  = getHeaderLength();
    unsigned fragment   = offset / (mtu - ipHeaderLen);
    unsigned packOffset = offset % (mtu - ipHeaderLen);

    const cEthernetPacket *packet = packetsAsArray[fragment];
    return packet->getPayloadAt16 (packOffset + (unsigned)ipHeaderLen);
}

size_t cIPv4Packet::getPayloadLength () const
{
    size_t ipHeaderLen = getHeaderLength();
    size_t len = 0;

    for (auto & p : packets)
    {
        BUG_ON (p.getPayloadLength () > ipHeaderLen);
        len += p.getPayloadLength () - ipHeaderLen;
    }

    return len;
}

void cIPv4Packet::updateL4Header (const uint8_t* l4header, size_t l4headerLen)
{
    cEthernetPacket &packet = packets.front();

    BUG_ON (packet.getPayloadLength () > getHeaderLength());

    packet.updatePayloadAt((unsigned)getHeaderLength(), l4header, l4headerLen);
}

void cIPv4Packet::updateHeaderChecksum ()
{
    ipHeader.chksum = 0;
    ipHeader.chksum = cInetChecksum::rfc1071((const uint16_t*)&ipHeader, getHeaderLength(), nullptr, 0);
}

void cIPv4Packet::addRouterAlertOption (void)
{
    ipHeader.setHeaderLenght(ipHeader.getHeaderLenght() + sizeof (ipv4_option_router_alert_t) / 4);
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cIPv4Packet::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    // FIXME unit tests!
}
#endif
