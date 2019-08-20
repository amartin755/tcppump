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


#ifndef ETHERNET_PACKET_H_
#define ETHERNET_PACKET_H_

#include <cstdint>
#include <cstddef>	// size_t
#include "protocoltypes.hpp"
#include "formatexception.hpp"
#include <cassert>

class cEthernetPacket
{
public:
	cEthernetPacket ();
	cEthernetPacket (size_t maxLength);
	cEthernetPacket (const cEthernetPacket& obj) = delete;
	~cEthernetPacket ();
	void setMacHeader (const mac_t& src, const mac_t& dest);
	void addLlcHeader (uint8_t dsap, uint8_t ssap, uint16_t control);
	void addSnapHeader (uint32_t oui, uint16_t protocol);
	void addVlanTag (bool isCTag, int id, int prio, int dei);
	void setTypeLength (uint16_t ethertypeLenth);
	void setLength ();
	void setPayload (const char* payload, size_t len);
	void setRaw (const char* payload, size_t len);
	const uint8_t* get ();
	inline size_t getLength () {return pPayload - packet + payloadLength;};
	inline void clear () {reset ();};

	static const size_t   MAX_ETHERNET_PAYLOAD     = 1500;
	static const size_t   MAX_PACKET               = 6+6+2+MAX_ETHERNET_PAYLOAD;
	static const size_t   MAX_TAGGED_PACKET        = MAX_PACKET + 4;
	static const size_t   MAX_DOUBLE_TAGGED_PACKET = MAX_TAGGED_PACKET + 4;


#ifdef WITH_UNITTESTS
	static void unitTest ();
#endif

private:
	void reset ();
	void updatePosition (size_t len);
	inline void checkPacketLength (size_t addedBytes)
	{
		if ((getLength () + addedBytes) > packetMaxLength)
			throw FormatException (exParRange, NULL);
	}

	const uint32_t* data;       // holds the packet data; do never access directly; use packet instead!
	uint8_t*  packet;			// always points to packet begin
	size_t    packetMaxLength;
	uint8_t*  pPayload;         // points at begin of payload (will be moved in case of tagging)
	uint16_t* pEthertypeLength; // points at ethertype/length field (will be moved in case of tagging)
	size_t    payloadLength;
	size_t    llcHeaderLength;

};

enum ethertypes_t
{
	ETHERTYPE_IPV4  = 0x0800,
	ETHERTYPE_ARP   = 0x0806,
	ETHERTYPE_CVLAN = 0x8100,
	ETHERTYPE_SVLAN = 0x88a8,
	ETHERTYPE_PN    = 0x8892,
};

#endif /* ETHERNET_PACKET_H_ */
