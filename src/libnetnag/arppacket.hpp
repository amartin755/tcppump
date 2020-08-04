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


#ifndef ARP_PACKET_H_
#define ARP_PACKET_H_

#include <cstdint>

#include "ethernetpacket.hpp"
#include "ipaddress.hpp"

class cArpPacket : public cEthernetPacket
{
public:
	cArpPacket ();
	void probe (mac_t srcMac, const cIpAddress& ip);
	void announce (mac_t srcMac, const cIpAddress& ip);
	void setAll (uint16_t opcode, mac_t srcMac, const cIpAddress& srcIp, mac_t dstMac, const cIpAddress& dstIp);



#ifdef WITH_UNITTESTS
	static void unitTest ();
#endif

private:

};

#pragma pack(1)

typedef struct
{
	uint16_t hwType;
	uint16_t protType;
	uint8_t  hwAddrSize;
	uint8_t  protAddrSize;
	uint16_t opcode;
	mac_t    srcMac;
	struct in_addr srcIp;
	mac_t    dstMac;
	struct in_addr dstIp;

}arp_t;

#pragma pack()

#endif /* ARP_PACKET_H_ */
