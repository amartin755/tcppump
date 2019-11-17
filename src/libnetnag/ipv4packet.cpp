/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2019 Andreas Martin (netnag@mailbox.org)
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


#include <cassert>
#include <cstring>

#include "inet.h"
#include "ipv4packet.hpp"



cIPv4Packet::cIPv4Packet (cEthernetPacket& p) : packet (p)
{
	memset (&header, 0, sizeof (header));
	header.setVersion (4);
	header.setHeaderLenght (5);

	packet.setTypeLength (ETHERTYPE_IPV4);
}

void cIPv4Packet::setDSCP (int dscp)
{
	header.setDSCP (dscp);
}

void cIPv4Packet::setECN (int ecn)
{
	header.setECN (ecn);
}

void cIPv4Packet::setTimeToLive (uint8_t ttl)
{
	header.ttl = ttl;
}

void cIPv4Packet::setDontFragment (bool df)
{
	header.setFlags (false, df, false);
}

void cIPv4Packet::setSource (ipv4_t ip)
{
	header.srcIp = ip;
}

void cIPv4Packet::setDestination (ipv4_t ip)
{
	header.dstIp = ip;
}

void cIPv4Packet::setPayload (uint8_t protocol, const char* payload, size_t len)
{
	header.len = htons (header.getHeaderLenght() * 4 + len / 2);
	header.protocol = protocol;
	updateHeaderChecksum();
	packet.setPayload ((uint8_t*)&header, sizeof (header));
	packet.appendPayload (payload, len);

	//TODO later when supporting fragmentation flags_offset and identification also have to be updated
}

void cIPv4Packet::updateHeaderChecksum ()
{
	header.chksum = htons(calcHeaderChecksum ((const uint16_t*)&header, sizeof (header)));
}

uint16_t cIPv4Packet::calcHeaderChecksum (const uint16_t* ipheader, int headerLen)
{
    assert (ipheader);
    assert (headerLen >= 20);
    assert (!(headerLen & 1));

    uint32_t chksum = 0;
    headerLen = headerLen / 2;

    while (headerLen--)
    {
        chksum += ntohs(*ipheader++);
    }

    chksum += (chksum >> 16) & 0x0f;

    return (uint16_t)(~chksum & 0x0000FFFF);
}

#ifdef WITH_UNITTESTS
void cIPv4Packet::unitTest ()
{
    uint8_t hd[] = {0x45,0x00,0x02,0x03,0x16,0xd1,0x00,0x00,0x01,0x11,0,0,0xc0,0xa8,0x00,0x88,0xef,0xff,0xff,0xfa};
    assert (cIPv4Packet::calcHeaderChecksum ((uint16_t*)hd, sizeof (hd)) == 0xefee);
    uint8_t hd1[] = {0x45,0x00,0x02,0x03,0x16,0xd1,0x00,0x00,0x01,0x11,0xef,0xee,0xc0,0xa8,0x00,0x88,0xef,0xff,0xff,0xfa};
    assert (cIPv4Packet::calcHeaderChecksum ((uint16_t*)hd1, sizeof (hd1)) == 0);
}
#endif
