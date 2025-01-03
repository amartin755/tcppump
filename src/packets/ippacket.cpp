// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2025 Andreas Martin (netnag@mailbox.org)
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
#include "ippacket.hpp"

#include "bug.hpp"
#include "settings.hpp"


uint16_t cIPPacket::v4::nextIdentification = 1;


cIPPacket::cIPPacket (bool isIPv6) : m_isIPv6 (isIPv6), m_mtu (cSettings::get().getMyMTU())
{
    cEthernetPacket firstPacket;
    firstPacket.setTypeLength (isIPv6 ? ETHERTYPE_IPV6 : ETHERTYPE_IPV4);
    m_packets.push_back(std::move(firstPacket));
    m_packetsAsArray = nullptr;

    m_dscp = 0;
    m_ecn  = 0;
    m_ttl  = 1;

    m_v4.hasId = false;
    m_v4.dontFragment = false;
    m_v4.identification = 0;
    m_v4.hasRouterAlertOption = false;

    m_v6.flowlabel = 0;
}

cIPPacket::~cIPPacket ()
{
    delete[] m_packetsAsArray;
    m_packetsAsArray = nullptr;
}

cEthernetPacket& cIPPacket::getFirstEthernetPacket ()
{
    BUG_ON (m_packets.size() <= 0);
    return m_packets.front();
}

std::list<cEthernetPacket>& cIPPacket::getAllEthernetPackets (void)
{
    return m_packets;
}

void cIPPacket::setDestMac (const cMacAddress& dest)
{
    for (auto & p : m_packets)
    {
        p.setDestMac(dest);
    }
}

void cIPPacket::setDSCP (unsigned dscp)
{
    m_dscp = dscp;
}

void cIPPacket::setECN (unsigned ecn)
{
    m_ecn = ecn;
}

void cIPPacket::setTimeToLive (uint8_t ttl)
{
    m_ttl = ttl;
}

void cIPPacket::setDontFragment (bool df)
{
    BUG_ON (m_isIPv6);
    m_v4.dontFragment = df;
}

void cIPPacket::setSource (const cIPv4& ip)
{
    BUG_ON (m_isIPv6);
    m_v4.srcIP = ip.get();
}

void cIPPacket::getSource (cIPv4& ip) const
{
    BUG_ON (m_isIPv6);
    ip.set (m_v4.srcIP);
}

void cIPPacket::setDestination (const cIPv4& ip)
{
    BUG_ON (m_isIPv6);
    m_v4.dstIP = ip.get();
}

void cIPPacket::getDestination (cIPv4& ip) const
{
    BUG_ON (m_isIPv6);
    ip.set (m_v4.dstIP);
}

void cIPPacket::setIdentification (uint16_t id)
{
    BUG_ON (m_isIPv6);
    m_v4.identification = id;
    m_v4.hasId = true;
}

void cIPPacket::setSource (const cIPv6& ip)
{
    BUG_ON (!m_isIPv6);
    m_v6.srcIP = ip.get();
}

void cIPPacket::getSource (cIPv6& ip) const
{
    BUG_ON (!m_isIPv6);
    ip.set (m_v6.srcIP);
}

void cIPPacket::setDestination (const cIPv6& ip)
{
    BUG_ON (!m_isIPv6);
    m_v6.dstIP = ip.get();
}

void cIPPacket::getDestination (cIPv6& ip) const
{
    BUG_ON (!m_isIPv6);
    ip.set (m_v6.dstIP);
}

void cIPPacket::setFlowLabel (unsigned fl)
{
    BUG_ON (!m_isIPv6);
    m_v6.flowlabel = fl;
}

void cIPPacket::v4compile (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen)
{
    if (l4headerLen + payloadLen + getHeaderLength() > 65535)
        throw FormatException (exParRange, nullptr);

    size_t ipHeaderLen = getHeaderLength();

    unsigned fragCnt = unsigned((l4headerLen + payloadLen - 1) / (m_mtu - ipHeaderLen)) + 1;
    size_t offset = 0;

    // we rely on L4 header fitting into first ip fragment
    BUG_ON (l4headerLen > m_mtu - ipHeaderLen);

    cEthernetPacket &packet = m_packets.front();

    // if there is no destination mac AND we have an ip multicast, translate to mac multicast
    if (!packet.hasDestMac())
    {
        cIPv4 dstIp (m_v4.dstIP);
        if (dstIp.isMulticast ())
        {
            packet.setDestMac (cMacAddress (1, 0, 0x5e, dstIp.getAsArray()[1] & 0x7f, dstIp.getAsArray()[2], dstIp.getAsArray()[3]));
        }
    }

    uint16_t id = 0;
    if (fragCnt > 1 && !m_v4.hasId)
        id = m_v4.nextIdentification++;
    else
        id = m_v4.identification;

    for (unsigned n = 1; n < fragCnt; n++)
        m_packets.push_back (cEthernetPacket(packet));

    m_packetsAsArray = new const cEthernetPacket*[fragCnt];

    unsigned n = 0;
    for (auto & p : m_packets)
    {
        m_packetsAsArray[n] = &p;

        size_t fragLen = 0;
        if (n == 0)
            fragLen = l4headerLen + payloadLen + ipHeaderLen > m_mtu ? m_mtu - ipHeaderLen : l4headerLen + payloadLen;
        else
            fragLen = payloadLen + ipHeaderLen > m_mtu ? m_mtu - ipHeaderLen : payloadLen;

        if ((n + 1) != m_packets.size())
        {
            fragLen /= 8;
            fragLen *= 8;
        }

        // write IP header
        ipv4_header_with_router_alert_t header;
        header.compile (
            m_v4.srcIP,
            m_v4.dstIP,
            m_ttl,
            protocol,
            m_dscp,
            m_ecn,
            m_v4.dontFragment,
            n + 1 != fragCnt,
            (unsigned)offset,
            (unsigned)ipHeaderLen,
            uint16_t(ipHeaderLen + fragLen),
            id,
            m_v4.hasRouterAlertOption);
        p.setPayload ((uint8_t*)&header, ipHeaderLen);

        if (n == 0)
        {
            fragLen -= l4headerLen;
            if (l4headerLen && l4header)
                p.appendPayload(l4header, l4headerLen);  // copy L4 header (e.g. udp header)
            if (payloadLen && payload)
                p.appendPayload (payload, fragLen);      // copy payload

            payloadLen -= fragLen;
            offset  += fragLen + l4headerLen;
        }
        else
        {
            p.appendPayload (payload, fragLen);          // copy payload
            payloadLen -= fragLen;
            offset  += fragLen;
        }

        payload += fragLen;
        n++;
    }
}

void cIPPacket::v6compile (uint8_t protocol, const uint8_t* l4header, size_t l4headerLen, const uint8_t* payload, size_t payloadLen)
{
    if (l4headerLen + payloadLen + getHeaderLength() > 65535)
        throw FormatException (exParRange, nullptr);

    size_t ipHeaderLen = getHeaderLength();

    unsigned fragCnt = unsigned((l4headerLen + payloadLen - 1) / (m_mtu - ipHeaderLen)) + 1;
    //FIXME IPv6 fragmentation not yet implemented
    BUG_ON (fragCnt > 1);
    size_t offset = 0;

    // we rely on L4 header fitting into first ip fragment
    BUG_ON (l4headerLen > m_mtu - ipHeaderLen);

    cEthernetPacket &packet = m_packets.front();

    // if there is no destination mac AND we have an ip multicast, translate to mac multicast
/*    if (!packet.hasDestMac())
    {
        cIPv4 dstIp (m_v4.dstIP);
        if (dstIp.isMulticast ())
        {
            packet.setDestMac (cMacAddress (1, 0, 0x5e, dstIp.getAsArray()[1] & 0x7f, dstIp.getAsArray()[2], dstIp.getAsArray()[3]));
        }
    }
*/

    for (unsigned n = 1; n < fragCnt; n++)
        m_packets.push_back (cEthernetPacket(packet));

    m_packetsAsArray = new const cEthernetPacket*[fragCnt];

    unsigned n = 0;
    for (auto & p : m_packets)
    {
        m_packetsAsArray[n] = &p;

        size_t fragLen = 0;
        if (n == 0)
            fragLen = l4headerLen + payloadLen + ipHeaderLen > m_mtu ? m_mtu - ipHeaderLen : l4headerLen + payloadLen;
        else
            fragLen = payloadLen + ipHeaderLen > m_mtu ? m_mtu - ipHeaderLen : payloadLen;

        if ((n + 1) != m_packets.size())
        {
            fragLen /= 8;
            fragLen *= 8;
        }

        // write IP header
        ipv6_header_t header;
        header.compile (
            m_v6.srcIP,
            m_v6.dstIP,
            m_ttl,
            protocol,
            m_dscp,
            m_ecn,
            m_v6.flowlabel,
            uint16_t(fragLen));
        p.setPayload ((uint8_t*)&header, ipHeaderLen);

        if (n == 0)
        {
            fragLen -= l4headerLen;
            if (l4headerLen && l4header)
                p.appendPayload(l4header, l4headerLen);  // copy L4 header (e.g. udp header)
            if (payloadLen && payload)
                p.appendPayload (payload, fragLen);      // copy payload

            payloadLen -= fragLen;
            offset  += fragLen + l4headerLen;
        }
        else
        {
            p.appendPayload (payload, fragLen);          // copy payload
            payloadLen -= fragLen;
            offset  += fragLen;
        }

        payload += fragLen;
        n++;
    }
}

size_t cIPPacket::getPayloadLength () const
{
    size_t ipHeaderLen = getHeaderLength();
    size_t len = 0;

    for (auto & p : m_packets)
    {
        BUG_ON (p.getPayloadLength () <= ipHeaderLen);
        len += p.getPayloadLength () - ipHeaderLen;
    }

    return len;
}

void cIPPacket::updateL4Header (const uint8_t* l4header, size_t l4headerLen)
{
    cEthernetPacket &packet = m_packets.front();

    BUG_ON (packet.getPayloadLength () <= getHeaderLength());

    packet.updatePayloadAt((unsigned)getHeaderLength(), l4header, l4headerLen);
}

void cIPPacket::addRouterAlertOption ()
{
    m_v4.hasRouterAlertOption = true;
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cIPPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
    {
        ipv4_header_with_router_alert_t h;
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv4 ("172.16.0.77").get (), cIPv4 ("224.0.0.251").get(), 255, 17, 0, 0, true, false, 0, 20, 88, 4064, false);
            const uint8_t bytes[] = {0x45, 0x0, 0x0, 0x58, 0xf, 0xe0, 0x40, 0x0, 0xff, 0x11, 0xde, 0x5b, 0xac, 0x10, 0x0, 0x4d, 0xe0, 0x0, 0x0, 0xfb};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv4 ("172.16.0.103").get (), cIPv4 ("172.16.0.1").get(), 64, 1, 63, 3, true, false, 0, 20, 84, 6450, false);
            const uint8_t bytes[] = {0x45, 0xff, 0x0, 0x54, 0x19, 0x32, 0x40, 0x0, 0x40, 0x1, 0xc7, 0xef, 0xac, 0x10, 0x0, 0x67, 0xac, 0x10, 0x0, 0x1};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv4 ("172.16.0.103").get (), cIPv4 ("172.16.0.1").get(), 64, 1, 1, 1, true, false, 0, 20, 84, 53330, false);
            const uint8_t bytes[] = {0x45, 0x5, 0x0, 0x54, 0xd0, 0x52, 0x40, 0x0, 0x40, 0x1, 0x11, 0xc9, 0xac, 0x10, 0x0, 0x67, 0xac, 0x10, 0x0, 0x1};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv4 ("172.16.0.103").get (), cIPv4 ("172.16.0.1").get(), 64, 1, 32, 2, true, false, 0, 20, 84, 13950, false);
            const uint8_t bytes[] = {0x45, 0x82, 0x0, 0x54, 0x36, 0x7e, 0x40, 0x0, 0x40, 0x1, 0xab, 0x20, 0xac, 0x10, 0x0, 0x67, 0xac, 0x10, 0x0, 0x1};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv4 ("172.16.0.103").get (), cIPv4 ("172.16.0.1").get(), 64, 1, 0, 0, false, true, 0, 20, 1500, 32647, false);
            const uint8_t bytes[] = {0x45, 0x0, 0x5, 0xdc, 0x7f, 0x87, 0x20, 0x0, 0x40, 0x1, 0x7d, 0x11, 0xac, 0x10, 0x0, 0x67, 0xac, 0x10, 0x0, 0x1};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv4 ("172.16.0.103").get (), cIPv4 ("172.16.0.1").get(), 64, 1, 0, 0, false, false, 1480, 20, 548, 32647, false);
            const uint8_t bytes[] = {0x45, 0x0, 0x2, 0x24, 0x7f, 0x87, 0x0, 0xb9, 0x40, 0x1, 0xa0, 0x10, 0xac, 0x10, 0x0, 0x67, 0xac, 0x10, 0x0, 0x1};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv4 ("172.16.0.1").get (), cIPv4 ("224.0.0.1").get(), 1, 2, 48, 0, true, false, 0, 24, 36, 0, true);
            const uint8_t bytes[] = {0x46, 0xc0, 0x0, 0x24, 0x0, 0x0, 0x40, 0x0, 0x1, 0x2, 0x58, 0x1, 0xac, 0x10, 0x0, 0x1, 0xe0, 0x0, 0x0, 0x1, 0x94, 0x4, 0x0, 0x0};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
    }
    {
        ipv6_header_t h;
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv6 ("fe80::e3cf:89ef:a7a0:1273").get (), cIPv6 ("ff02::2").get(), 255, 58, 0, 0, 0x5fbda, 8);
            const uint8_t bytes[] = {0x60, 0x5, 0xfb, 0xda, 0x0, 0x8, 0x3a, 0xff, 0xfe, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe3, 0xcf, 0x89, 0xef, 0xa7, 0xa0, 0x12, 0x73, 0xff, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv6 ("1234::1").get (), cIPv6 ("5678::1").get(), 64, 58, 0, 0, 0x12345, 64);
            const uint8_t bytes[] = {0x60, 0x1, 0x23, 0x45, 0x0, 0x40, 0x3a, 0x40, 0x12, 0x34, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1, 0x56, 0x78, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv6 ("1234::1").get (), cIPv6 ("5678::1").get(), 64, 58, 63, 3, 0x12345, 64);
            const uint8_t bytes[] = {0x6f, 0xf1, 0x23, 0x45, 0x0, 0x40, 0x3a, 0x40, 0x12, 0x34, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1, 0x56, 0x78, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
        {
            std::memset (&h, 0xff, sizeof (h));
            h.compile (cIPv6 ("1234::1").get (), cIPv6 ("5678::1").get(), 64, 58, 63, 1, 0x12345, 64);
            const uint8_t bytes[] = {0x6f, 0xd1, 0x23, 0x45, 0x0, 0x40, 0x3a, 0x40, 0x12, 0x34, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1, 0x56, 0x78, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1};
            BUG_ON (std::memcmp (&h, bytes, sizeof (bytes)));
        }
    }
    {
        const uint8_t ipheader[] = {0x6f, 0xd1, 0x23, 0x45, 0x0, 0x40, 0x3a, 0x40, 0x12, 0x34, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1, 0x56, 0x78, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1};
        const uint8_t ippayload[] = {0x80, 0x0, 0x44, 0x12, 0x0, 0x2, 0x0, 0x1, 0x37, 0xb9, 0x77, 0x67, 0x0, 0x0, 0x0, 0x0, 0x99, 0x7a, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
        uint8_t payload[sizeof (ipheader) + sizeof (ippayload)];
        std::memcpy (payload, ipheader, sizeof (ipheader));
        std::memcpy (payload + sizeof(ipheader), ippayload, sizeof (ippayload));

        cIPPacket obj (true);
        obj.setSource (cIPv6 ("1234::1").get ());
        obj.setDestination (cIPv6 ("5678::1").get());
        obj.setDSCP (63);
        obj.setECN (1);
        obj.setTimeToLive (64);
        obj.setFlowLabel (0x12345);
        obj.compile (PROTO_ICMPv6, nullptr, 0, ippayload, sizeof (ippayload));

        BUG_ON (obj.m_packetsAsArray[0]->getPayloadLength() != sizeof (payload));
        BUG_ON (std::memcmp (obj.m_packetsAsArray[0]->getPayload(), payload, sizeof (payload)));
    }
    {
        const uint8_t ipheader[]  = {0x6f, 0xd1, 0x23, 0x45, 0x0, 0x40, 0x3a, 0x40, 0x12, 0x34, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1, 0x56, 0x78, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, 0, 0, 0, 0, 0, 0, 1};
        const uint8_t l4header[]  = {0x80, 0x0, 0x44, 0x12, 0x0, 0x2, 0x0, 0x1, 0x37, 0xb9, 0x77, 0x67, 0x0, 0x0, 0x0, 0x0, 0x99, 0x7a, 0x6, 0x0, 0x0, 0x0, 0x0, 0x0};
        const uint8_t l4payload[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
        uint8_t payload[sizeof (ipheader) + sizeof (l4header) + sizeof (l4payload)];
        std::memcpy (payload, ipheader, sizeof (ipheader));
        std::memcpy (payload + sizeof(ipheader), l4header, sizeof (l4header));
        std::memcpy (payload + sizeof(ipheader) + sizeof (l4header), l4payload, sizeof (l4payload));

        cIPPacket obj (true);
        obj.setSource (cIPv6 ("1234::1").get ());
        obj.setDestination (cIPv6 ("5678::1").get());
        obj.setDSCP (63);
        obj.setECN (1);
        obj.setTimeToLive (64);
        obj.setFlowLabel (0x12345);
        obj.compile (PROTO_ICMPv6, l4header, sizeof(l4header), l4payload, sizeof (l4payload));

        BUG_ON (obj.m_packetsAsArray[0]->getPayloadLength() != sizeof (payload));
        BUG_ON (std::memcmp (obj.m_packetsAsArray[0]->getPayload(), payload, sizeof (payload)));
    }
}
#endif
