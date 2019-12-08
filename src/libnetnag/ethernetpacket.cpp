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

#include "ethernetpacket.hpp"

#include "converter.hpp"



cEthernetPacket::cEthernetPacket () : cEthernetPacket (MAX_DOUBLE_TAGGED_PACKET)
{
}


cEthernetPacket::cEthernetPacket (size_t maxLength)
{
	assert (maxLength >= sizeof (mac_header_t));

	/*
	 * note: We use an array of uint32_t here to force 32bit aligment of our packet data.
	 * Yes, that's paranoid. I know that 'new' always aligns to __STDCPP_DEFAULT_NEW_ALIGNMENT__, which is
	 * typically 8 or 16 bytes, but you never know ;-)
	 */
	data = new uint32_t[(maxLength + sizeof (uint32_t) - 1) / sizeof (uint32_t)];

	packet          = (uint8_t*)data;
	packetMaxLength = maxLength;

	reset ();
}

cEthernetPacket::cEthernetPacket (cEthernetPacket&& other)
{
	data             = other.data;
	packet           = other.packet;
	packetMaxLength  = other.packetMaxLength;
	pPayload         = other.pPayload;
	pEthertypeLength = other.pEthertypeLength;
	payloadLength    = other.payloadLength;
	llcHeaderLength  = other.llcHeaderLength;
	vlanTags         = other.vlanTags;

	other.data             = nullptr;
	other.packet           = nullptr;
	other.packetMaxLength  = 0;
	other.pPayload         = nullptr;
	other.pEthertypeLength = nullptr;
	other.payloadLength    = 0;
	other.llcHeaderLength  = 0;
}


cEthernetPacket::~cEthernetPacket ()
{
	delete[] data;
}


cEthernetPacket& cEthernetPacket::operator=(cEthernetPacket&& other)
{
	if (this != &other)
	{
		delete[] data;

		data             = other.data;
		packet           = other.packet;
		packetMaxLength  = other.packetMaxLength;
		pPayload         = other.pPayload;
		pEthertypeLength = other.pEthertypeLength;
		payloadLength    = other.payloadLength;
		llcHeaderLength  = other.llcHeaderLength;
		vlanTags         = other.vlanTags;

		other.data             = nullptr;
		other.packet           = nullptr;
		other.packetMaxLength  = 0;
		other.pPayload         = nullptr;
		other.pEthertypeLength = nullptr;
		other.payloadLength    = 0;
		other.llcHeaderLength  = 0;
	}

	return *this;
}


void cEthernetPacket::reset ()
{
	pPayload          = packet;
	pPayload         += sizeof (mac_header_t);
	pEthertypeLength  = (uint16_t*)(&((mac_header_t*)packet)->ethertypeLength);
	payloadLength     = 0;
	llcHeaderLength   = 0;
	vlanTags          = 0;
	*pEthertypeLength = 0;
}


void cEthernetPacket::updatePosition (size_t len)
{
	pPayload += len;
	pEthertypeLength = (uint16_t*)((uint8_t*)pEthertypeLength + len);
}


void cEthernetPacket::setTypeLength (uint16_t ethertypeLength)
{
	*pEthertypeLength = htons (ethertypeLength);
}


void cEthernetPacket::setLength ()
{
	setTypeLength (payloadLength + llcHeaderLength);
}


void cEthernetPacket::setMacHeader (const mac_t& src, const mac_t& dest)
{
	// mac header contains source and destination mac and is always at the begin of the packet
	mac_header_t* header = (mac_header_t*)packet;
	header->src  = src;
	header->dest = dest;
}


void cEthernetPacket::addLlcHeader (uint8_t dsap, uint8_t ssap, uint16_t control)
{
	// size of the control word can either be 8 or 16 bits (depends on its content)
	llcHeaderLength = (control & 0x0003) == 3 ?
			sizeof (llc_t) - 1 : sizeof (llc_t);

	checkPacketLength (llcHeaderLength);

	// if there is already payload, we move it
	if (payloadLength)
	{
		memmove (pPayload + llcHeaderLength, pPayload, payloadLength);
	}

	llc_t* llc = (llc_t*)pPayload;

	llc->dsap = dsap;
	llc->ssap = ssap;

	if ((control & 0x0003) == 3)
	{
		llc->control.c8 = (uint8_t)control;
	}
	else
	{
		llc->control.c16 = htons (control);
	}

	pPayload += llcHeaderLength;
	setLength ();
}


void cEthernetPacket::addSnapHeader (uint32_t oui, uint16_t protocol)
{
	addLlcHeader (0xaa, 0xaa, 3);

	checkPacketLength (sizeof (snap_t));

	// if there is already payload, we move it
	if (payloadLength)
	{
		memmove (pPayload + sizeof (snap_t), pPayload, payloadLength);
	}

	llcHeaderLength += sizeof (snap_t);
	oui = htonl (oui);

	snap_t* snap   = (snap_t*)pPayload;
	snap->oui.a    = uint8_t((oui >>  8) & 0x000000ff);
	snap->oui.b    = uint8_t((oui >> 16) & 0x000000ff);
	snap->oui.c    = uint8_t((oui >> 24) & 0x000000ff);
	snap->protocol = htons (protocol);

	pPayload += sizeof (snap_t);
	setLength ();
}


void cEthernetPacket::addVlanTag (bool isCTag, int id, int prio, int dei)
{
	checkPacketLength (sizeof (vlan_t));

	vlan_t* tag = (vlan_t*)pEthertypeLength;

	memmove ((uint8_t*)pEthertypeLength + sizeof (vlan_t), pEthertypeLength, 2 + payloadLength);
	isCTag ? tag->setCTag (id, prio, dei) : tag->setSTag (id, prio, dei);
	updatePosition (sizeof (vlan_t));
	vlanTags++;
}


void cEthernetPacket::setPayload (const char* payloadAsHexStr, size_t len)
{
	payloadLength = 0;
	checkPacketLength ((len + 1)/2);

	int copiedLen = packet + packetMaxLength - pPayload;
	if (!nn::Converter::hexStringToBin (payloadAsHexStr, len, pPayload, &copiedLen))
		throw FormatException (exParFormat, payloadAsHexStr);

	payloadLength = copiedLen;
}


void cEthernetPacket::setPayload (const uint8_t* payload, size_t len)
{
	payloadLength = 0;
	checkPacketLength (len);
	memcpy (pPayload, payload, len);
	payloadLength = len;
}


void cEthernetPacket::appendPayload (const char* payloadAsHexStr, size_t len)
{
	checkPacketLength ((len + 1)/2);

	uint8_t* p = pPayload + payloadLength;

	int copiedLen = packet + packetMaxLength - p;
	if (!nn::Converter::hexStringToBin (payloadAsHexStr, len, p, &copiedLen))
		throw FormatException (exParFormat, payloadAsHexStr);

	payloadLength += copiedLen;
}


void cEthernetPacket::setRaw (const char* payloadAsHexStr, size_t len)
{
	if ((len + 1)/2 > packetMaxLength)
		throw FormatException (exParRange, NULL);

	reset ();

	int copiedLen = packetMaxLength;
	if (!nn::Converter::hexStringToBin (payloadAsHexStr, len, packet, &copiedLen))
		throw FormatException (exParFormat, payloadAsHexStr);

	payloadLength = copiedLen - sizeof (mac_header_t);
}


void cEthernetPacket::setRaw (const uint8_t* payload, size_t len)
{
	reset ();
	checkPacketLength (len);
	memcpy (packet, payload, len);
	payloadLength = len - sizeof (mac_header_t);
}


const uint8_t* cEthernetPacket::get ()
{
	return packet;
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cEthernetPacket::unitTest ()
{
	nn::Console::PrintDebug("-- " __FILE__ " --\n");

	mac_t src = {0x12,0x34,0x56,0x78,0x9a,0xbc};
	mac_t dst = {0x11,0x22,0x33,0x44,0x55,0x66};


	try
	{
		cEthernetPacket obj(MAX_DOUBLE_TAGGED_PACKET + 1);
		memset (obj.packet, 0xcc, MAX_DOUBLE_TAGGED_PACKET + 1);
		obj.clear();

		obj.setMacHeader(src, dst);
		assert (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x00\x00\xcc\xcc", 16));
		assert (obj.getLength() == 14);
		obj.setLength ();
		assert (obj.getLength() == 14);
		assert (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x00\x00\xcc\xcc", 16));
		obj.setTypeLength (0x1234);
		assert (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x12\x34\xcc\xcc", 16));
		assert (obj.getLength() == 14);
		obj.addVlanTag(false, 12, 7, 0);
		assert (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x88\xa8\xe0\x0c\x12\x34\xcc\xcc", 20));
		assert (obj.getLength() == 18);
		obj.setPayload ("aabbccddeeff0a0b0c0d0e0f", 24);
		assert (obj.getLength() == 30);
		assert (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x88\xa8\xe0\x0c\x12\x34\xaa\xbb\xcc\xdd\xee\xff\x0a\x0b\x0c\x0d\x0e\x0f\xcc\xcc", 32));
		obj.addVlanTag(true, 12, 7, 0);
		assert (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x88\xa8\xe0\x0c\x81\x00\xe0\x0c\x12\x34\xaa\xbb\xcc\xdd\xee\xff\x0a\x0b\x0c\x0d\x0e\x0f\xcc\xcc", 36));
		assert (obj.getLength() == 34);
		obj.setLength();
		assert (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x88\xa8\xe0\x0c\x81\x00\xe0\x0c\x00\x0c\xaa\xbb\xcc\xdd\xee\xff\x0a\x0b\x0c\x0d\x0e\x0f\xcc\xcc", 36));
		obj.addLlcHeader(0x10, 0x20, 3);
		assert (obj.getLength() == 37);

		memset (obj.packet, 0xcc, MAX_DOUBLE_TAGGED_PACKET + 1);
		obj.reset();
		obj.setMacHeader(src, dst);
		obj.addSnapHeader(0x00808182, 0x9876);
		assert (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x00\x08\xaa\xaa\x03\x80\x81\x82\x98\x76\xcc\xcc", 24));
		assert (obj.getLength() == 22);
	}
	catch (FormatException& e)
	{
		assert (0);
	}

	bool catched = false;
	try
	{
		cEthernetPacket obj(sizeof (mac_header_t) + sizeof(vlan_t));
		obj.addVlanTag(false, 12, 7, 0);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	try
	{
		catched = false;
		cEthernetPacket obj(sizeof (mac_header_t) + sizeof(vlan_t)-1);
		obj.addVlanTag(false, 12, 7, 0);
	}
	catch (FormatException& e)
	{
		catched = true;
	}
	assert (catched);

	try
	{
		cEthernetPacket obj(sizeof (mac_header_t) + 2*sizeof(vlan_t));
		obj.addVlanTag(false, 12, 7, 0);
		obj.addVlanTag(true, 12, 7, 0);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	try
	{
		catched = false;
		cEthernetPacket obj(sizeof (mac_header_t) + 2*sizeof(vlan_t)-1);
		obj.addVlanTag(false, 12, 7, 0);
		obj.addVlanTag(true, 12, 7, 0);
	}
	catch (FormatException& e)
	{
		catched = true;
	}
	assert (catched);

	try
	{
		cEthernetPacket obj(sizeof (mac_header_t));
		obj.setRaw("12345678901234567890aabbccdd", 28);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	try
	{
		catched = false;
		cEthernetPacket obj(sizeof (mac_header_t));
		obj.setRaw("12345678901234567890aabbccddee", 30);
	}
	catch (FormatException& e)
	{
		catched = true;
	}
	assert (catched);

	try
	{
		cEthernetPacket obj(sizeof (mac_header_t) + 1);
		obj.setPayload ("aa", 2);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	try
	{
		catched = false;
		cEthernetPacket obj(sizeof (mac_header_t) + 1);
		obj.setPayload ("aabb", 4);
	}
	catch (FormatException& e)
	{
		catched = true;
	}
	assert (catched);

	try
	{
		cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t));
		obj.addLlcHeader(12, 34, 0);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	try
	{
		catched = false;
		cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t)-1);
		obj.addLlcHeader(12, 34, 0);
	}
	catch (FormatException& e)
	{
		catched = true;
	}
	assert (catched);

	try
	{
		cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t) - 1);
		obj.addLlcHeader(12, 34, 3);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	try
	{
		catched = false;
		cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t) - 1 - 1);
		obj.addLlcHeader(12, 34, 3);
	}
	catch (FormatException& e)
	{
		catched = true;
	}
	assert (catched);

	try
	{
		cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t) + sizeof(snap_t) - 1);
		obj.addSnapHeader(0x123456, 1234);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	try
	{
		catched = false;
		cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t) + sizeof(snap_t) - 1 - 1);
		obj.addSnapHeader(0x123456, 1234);
	}
	catch (FormatException& e)
	{
		catched = true;
	}
	assert (catched);

	try
	{
		char payload[(MAX_DOUBLE_TAGGED_PACKET - 30)*2];
		memset (payload, '0', sizeof (payload));
		cEthernetPacket obj(MAX_DOUBLE_TAGGED_PACKET+1);
		memset (obj.packet, 0xcc, MAX_DOUBLE_TAGGED_PACKET+1);
		obj.reset();
		obj.addVlanTag(false, 12, 7, 0);
		assert (obj.packet[18] == 0xcc);
		obj.addVlanTag(true, 12, 7, 0);
		assert (obj.packet[22] == 0xcc);
		obj.addSnapHeader(0x123456, 1234);
		assert (obj.packet[30] == 0xcc);
		obj.setPayload (payload, sizeof (payload));
		assert (obj.packet[30] == 0);
		assert (obj.packet[MAX_DOUBLE_TAGGED_PACKET-1] == 0);
		assert (obj.packet[MAX_DOUBLE_TAGGED_PACKET] == 0xcc);
	}
	catch (FormatException& e)
	{
		assert (0);
	}
	try
	{
		catched = false;
		char payload[(MAX_DOUBLE_TAGGED_PACKET - 29)*2];
		memset (payload, '0', sizeof (payload));
		cEthernetPacket obj(MAX_DOUBLE_TAGGED_PACKET);
		memset (obj.packet, 0xcc, MAX_DOUBLE_TAGGED_PACKET);
		obj.reset();
		obj.addVlanTag(false, 12, 7, 0);
		assert (obj.packet[18] == 0xcc);
		obj.addVlanTag(true, 12, 7, 0);
		assert (obj.packet[22] == 0xcc);
		obj.addSnapHeader(0x123456, 1234);
		assert (obj.packet[30] == 0xcc);
		obj.setPayload (payload, sizeof (payload));
		assert (obj.packet[30] == 0);
		assert (obj.packet[MAX_DOUBLE_TAGGED_PACKET-1] == 0);
	}
	catch (FormatException& e)
	{
		catched = true;
	}
	assert (catched);
}
#endif
