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

#include "arppacket.hpp"



cArpPacket::cArpPacket (cEthernetPacket& p) : packet (p)
{

}


void cArpPacket::probe (mac_t srcMac, ipv4_t ip)
{
	mac_t dstMac;
	dstMac.set (0);
	set (1, srcMac, (ipv4_t)0, dstMac, ip);
}


void cArpPacket::announce (mac_t srcMac, ipv4_t ip)
{
	mac_t dstMac;
	dstMac.set (0);
	set (1, srcMac, ip, dstMac, ip);
}


void cArpPacket::set (uint16_t opcode, mac_t srcMac, ipv4_t srcIp, mac_t dstMac, ipv4_t dstIp)
{
	arp_t a;
	a.hwType       = htons (1);
	a.protType     = htons (ETHERTYPE_IPV4);
	a.hwAddrSize   = (uint8_t)sizeof (mac_t);
	a.protAddrSize = (uint8_t)sizeof (ipv4_t);
	a.opcode       = htons (opcode);
	a.srcMac       = srcMac;
	a.srcIp        = srcIp;
	a.dstMac       = dstMac;
	a.dstIp        = dstIp;

	if (dstMac.isNull())
		dstMac.set (0xffu);

	packet.setMacHeader (srcMac, dstMac);
	packet.setTypeLength (ETHERTYPE_ARP);
	packet.setPayload ((uint8_t*)&a, sizeof(a));
}


#ifdef WITH_UNITTESTS
void cArpPacket::unitTest ()
{
}
#endif
