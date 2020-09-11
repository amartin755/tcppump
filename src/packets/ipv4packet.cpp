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
#include "ipv4packet.hpp"
#include "inetchecksum.hpp"



cIPv4Packet::cIPv4Packet ()
{
    memset (&ipHeader, 0, sizeof (ipHeader));
    ipHeader.setVersion (4);
    ipHeader.setHeaderLenght (5);

    cEthernetPacket firstPacket;
    firstPacket.setTypeLength (ETHERTYPE_IPV4);
    packets.push_back(std::move(firstPacket));
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
    ipHeader.setFlags (false, df, false);
}

void cIPv4Packet::setSource (const cIpAddress& ip)
{
    ipHeader.srcIp = ip.get();
}

void cIPv4Packet::setDestination (const cIpAddress& ip)
{
    ipHeader.dstIp = ip.get();
}

void cIPv4Packet::setPayload (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen)
{
    // FIXME fragmentation

    ipHeader.len = htons (uint16_t(ipHeader.getHeaderLenght() * 4 + l4headerLen + payloadLen));
    ipHeader.protocol = protocol;
    updateHeaderChecksum();

    cEthernetPacket &packet = packets.front();

    packet.setPayload ((uint8_t*)&ipHeader, sizeof (ipHeader)); // write IP header
    if (l4headerLen && l4header)
        packet.appendPayload(l4header, l4headerLen);            // copy L4 header (e.g. udp header)
    if (payloadLen && payload)
        packet.appendPayload (payload, payloadLen);             // copy payload from ascii string

    //TODO later when supporting fragmentation flags_offset and identification also have to be updated
}

uint8_t cIPv4Packet::getPayloadAt8 (unsigned offset) const
{
    // FIXME fragmentation

    const cEthernetPacket &packet = packets.front();
    return packet.getPayloadAt8 (offset + sizeof (ipHeader));
}

// note: offset is a byte offset!!!
uint16_t cIPv4Packet::getPayloadAt16 (unsigned offset) const
{
    // FIXME fragmentation

    const cEthernetPacket &packet = packets.front();
    return packet.getPayloadAt16 (offset + sizeof (ipHeader));
}

size_t cIPv4Packet::getPayloadLength () const
{
    // FIXME fragmentation

    const cEthernetPacket &packet = packets.front();
    size_t len = packet.getPayloadLength ();

    BUG_ON (len > sizeof (ipHeader));

    return len > sizeof (ipHeader) ? len - sizeof (ipHeader) : 0;
}

void cIPv4Packet::updateL4Header (const uint8_t* l4header, size_t l4headerLen)
{
    cEthernetPacket &packet = packets.front();

    BUG_ON (packet.getPayloadLength () > sizeof (ipHeader));

    packet.updatePayloadAt((unsigned)sizeof (ipHeader), l4header, l4headerLen);
}

void cIPv4Packet::updateHeaderChecksum ()
{
    ipHeader.chksum = cInetChecksum::rfc1071((const uint16_t*)&ipHeader, sizeof (ipHeader), nullptr, 0);
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cIPv4Packet::unitTest ()
{
    nn::Console::PrintDebug("-- " __FILE__ " --\n");

    // FIXME unit tests!
}
#endif
