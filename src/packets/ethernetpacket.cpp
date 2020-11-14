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

#include "ethernetpacket.hpp"
#include "bug.hpp"



cEthernetPacket::cEthernetPacket () : cEthernetPacket (MAX_DOUBLE_TAGGED_PACKET)
{
}


cEthernetPacket::cEthernetPacket (size_t maxLength)
{
    BUG_ON (maxLength >= sizeof (mac_header_t));

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

// move constructor
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

// copy constructor
cEthernetPacket::cEthernetPacket (const cEthernetPacket& obj) : cEthernetPacket (obj.packetMaxLength)
{
    BUG_ON (packetMaxLength == obj.packetMaxLength);

    payloadLength    = obj.payloadLength;
    llcHeaderLength  = obj.llcHeaderLength;
    vlanTags         = obj.vlanTags;

    // copy packet data
    std::memcpy (packet, obj.packet, obj.getLength());

    // recalculate offsets
    pPayload         = packet + (obj.pPayload - obj.packet);
    pEthertypeLength = (uint16_t*)(packet + ((uint8_t*)obj.pEthertypeLength - obj.packet));
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
    setTypeLength (uint16_t(payloadLength + llcHeaderLength));
}


void cEthernetPacket::setMacHeader (const cMacAddress& src, const cMacAddress& dest)
{
    setSrcMac (src);
    setDestMac (dest);
}


void cEthernetPacket::setDestMac (const cMacAddress& dest)
{
    // mac header contains source and destination mac and is always at the begin of the packet
    mac_header_t* header = (mac_header_t*)packet;
    std::memcpy(&header->dest, dest.get(), dest.size());
}


void cEthernetPacket::setSrcMac (const cMacAddress& src)
{
    // mac header contains source and destination mac and is always at the begin of the packet
    mac_header_t* header = (mac_header_t*)packet;
    std::memcpy(&header->src, src.get(), src.size());
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
        ::memmove (pPayload + llcHeaderLength, pPayload, payloadLength);
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
        ::memmove (pPayload + sizeof (snap_t), pPayload, payloadLength);
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


void cEthernetPacket::setPayload (const uint8_t* payload, size_t len)
{
    payloadLength = 0;
    checkPacketLength (len);
    std::memcpy (pPayload, payload, len);
    payloadLength = len;
}


void cEthernetPacket::appendPayload (const uint8_t* payload, size_t len)
{
    checkPacketLength (len);
    uint8_t* p = pPayload + payloadLength;
    std::memcpy (p, payload, len);
    payloadLength += len;
}


void cEthernetPacket::setRaw (const uint8_t* payload, size_t len)
{
    reset ();
    if (len > packetMaxLength)
        throw FormatException (exParRange, NULL);
    std::memcpy (packet, payload, len);
    payloadLength = len - sizeof (mac_header_t);
}


const uint8_t* cEthernetPacket::get () const
{
    return packet;
}


void cEthernetPacket::updatePayloadAt (unsigned offset, const void* payload, size_t len)
{
    if ((offset + len) > payloadLength)
        throw FormatException (exParRange, NULL);

    std::memcpy (&(pPayload[offset]), payload, len);
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cEthernetPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    cMacAddress src("12:34:56:78:9a:bc");
    cMacAddress dst("11:22:33:44:55:66");


    try
    {
        cEthernetPacket obj(MAX_DOUBLE_TAGGED_PACKET + 1);
        memset (obj.packet, 0xcc, MAX_DOUBLE_TAGGED_PACKET + 1);
        obj.clear();

        obj.setMacHeader(src, dst);
        BUG_ON (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x00\x00\xcc\xcc", 16));
        BUG_ON (obj.getLength() == 14);
        obj.setLength ();
        BUG_ON (obj.getLength() == 14);
        BUG_ON (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x00\x00\xcc\xcc", 16));
        obj.setTypeLength (0x1234);
        BUG_ON (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x12\x34\xcc\xcc", 16));
        BUG_ON (obj.getLength() == 14);
        obj.addVlanTag(false, 12, 7, 0);
        BUG_ON (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x88\xa8\xe0\x0c\x12\x34\xcc\xcc", 20));
        BUG_ON (obj.getLength() == 18);
        obj.setPayload ((uint8_t*)"\xaa\xbb\xcc\xdd\xee\xff\x0a\x0b\x0c\x0d\x0e\x0f", 12);
        BUG_ON (obj.getLength() == 30);
        BUG_ON (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x88\xa8\xe0\x0c\x12\x34\xaa\xbb\xcc\xdd\xee\xff\x0a\x0b\x0c\x0d\x0e\x0f\xcc\xcc", 32));
        obj.addVlanTag(true, 12, 7, 0);
        BUG_ON (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x88\xa8\xe0\x0c\x81\x00\xe0\x0c\x12\x34\xaa\xbb\xcc\xdd\xee\xff\x0a\x0b\x0c\x0d\x0e\x0f\xcc\xcc", 36));
        BUG_ON (obj.getLength() == 34);
        obj.setLength();
        BUG_ON (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x88\xa8\xe0\x0c\x81\x00\xe0\x0c\x00\x0c\xaa\xbb\xcc\xdd\xee\xff\x0a\x0b\x0c\x0d\x0e\x0f\xcc\xcc", 36));
        obj.addLlcHeader(0x10, 0x20, 3);
        BUG_ON (obj.getLength() == 37);

        {
            // test copy constructor
            cEthernetPacket cpy(obj);
            BUG_ON (obj.data != cpy.data);
            BUG_ON (obj.packet != cpy.packet);
            BUG_ON (obj.pPayload != cpy.pPayload);
            BUG_ON (obj.pEthertypeLength != cpy.pEthertypeLength);
            BUG_ON (obj.packetMaxLength == cpy.packetMaxLength);
            BUG_ON (obj.payloadLength == cpy.payloadLength);
            BUG_ON (obj.llcHeaderLength == cpy.llcHeaderLength);
            BUG_ON (obj.vlanTags == cpy.vlanTags);
            BUG_ON (*obj.data == *cpy.data);
            BUG_ON (*obj.packet == *cpy.packet);
            BUG_ON (*obj.pPayload == *cpy.pPayload);
            BUG_ON (*obj.pEthertypeLength == *cpy.pEthertypeLength);
            BUG_ON (!memcmp (obj.packet, cpy.packet, (obj.pPayload + obj.payloadLength) - obj.packet));
        }

        memset (obj.packet, 0xcc, MAX_DOUBLE_TAGGED_PACKET + 1);
        obj.reset();
        obj.setMacHeader(src, dst);
        obj.addSnapHeader(0x00808182, 0x9876);
        BUG_ON (!memcmp (obj.packet, "\x11\x22\x33\x44\x55\x66\x12\x34\x56\x78\x9a\xbc\x00\x08\xaa\xaa\x03\x80\x81\x82\x98\x76\xcc\xcc", 24));
        BUG_ON (obj.getLength() == 22);

        {
            // test copy constructor
            cEthernetPacket cpy(obj);
            BUG_ON (obj.data != cpy.data);
            BUG_ON (obj.packet != cpy.packet);
            BUG_ON (obj.pPayload != cpy.pPayload);
            BUG_ON (obj.pEthertypeLength != cpy.pEthertypeLength);
            BUG_ON (obj.packetMaxLength == cpy.packetMaxLength);
            BUG_ON (obj.payloadLength == cpy.payloadLength);
            BUG_ON (obj.llcHeaderLength == cpy.llcHeaderLength);
            BUG_ON (obj.vlanTags == cpy.vlanTags);
            BUG_ON (*obj.data == *cpy.data);
            BUG_ON (*obj.packet == *cpy.packet);
            BUG_ON (*obj.pEthertypeLength == *cpy.pEthertypeLength);
            BUG_ON (!memcmp (obj.packet, cpy.packet, obj.getLength()));
        }
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }

    bool catched = false;
    try
    {
        cEthernetPacket obj(sizeof (mac_header_t) + sizeof(vlan_t));
        obj.addVlanTag(false, 12, 7, 0);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    try
    {
        catched = false;
        cEthernetPacket obj(sizeof (mac_header_t) + sizeof(vlan_t)-1);
        obj.addVlanTag(false, 12, 7, 0);
    }
    catch (FormatException& )
    {
        catched = true;
    }
    BUG_ON (catched);

    try
    {
        cEthernetPacket obj(sizeof (mac_header_t) + 2*sizeof(vlan_t));
        obj.addVlanTag(false, 12, 7, 0);
        obj.addVlanTag(true, 12, 7, 0);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    try
    {
        catched = false;
        cEthernetPacket obj(sizeof (mac_header_t) + 2*sizeof(vlan_t)-1);
        obj.addVlanTag(false, 12, 7, 0);
        obj.addVlanTag(true, 12, 7, 0);
    }
    catch (FormatException& )
    {
        catched = true;
    }
    BUG_ON (catched);

    try
    {
        cEthernetPacket obj(sizeof (mac_header_t));
        obj.setRaw((uint8_t*)"\x12\x34\x56\x78\x90\x12\x34\x56\x78\x90\xaa\xbb\xcc\xdd", 14);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    try
    {
        catched = false;
        cEthernetPacket obj(sizeof (mac_header_t));
        obj.setRaw((uint8_t* )"\x12\x34\x56\x78\x90\x12\x34\x56\x78\x90\xaa\xbb\xcc\xdd\xee", 15);
    }
    catch (FormatException& )
    {
        catched = true;
    }
    BUG_ON (catched);

    try
    {
        cEthernetPacket obj(sizeof (mac_header_t) + 1);
        obj.setPayload ((uint8_t* )"\xaa", 1);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    try
    {
        catched = false;
        cEthernetPacket obj(sizeof (mac_header_t) + 1);
        obj.setPayload ((uint8_t* )"\xaa\xbb", 2);
    }
    catch (FormatException& )
    {
        catched = true;
    }
    BUG_ON (catched);

    try
    {
        cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t));
        obj.addLlcHeader(12, 34, 0);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    try
    {
        catched = false;
        cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t)-1);
        obj.addLlcHeader(12, 34, 0);
    }
    catch (FormatException& )
    {
        catched = true;
    }
    BUG_ON (catched);

    try
    {
        cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t) - 1);
        obj.addLlcHeader(12, 34, 3);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    try
    {
        catched = false;
        cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t) - 1 - 1);
        obj.addLlcHeader(12, 34, 3);
    }
    catch (FormatException& )
    {
        catched = true;
    }
    BUG_ON (catched);

    try
    {
        cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t) + sizeof(snap_t) - 1);
        obj.addSnapHeader(0x123456, 1234);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    try
    {
        catched = false;
        cEthernetPacket obj(sizeof (mac_header_t) + sizeof(llc_t) + sizeof(snap_t) - 1 - 1);
        obj.addSnapHeader(0x123456, 1234);
    }
    catch (FormatException& )
    {
        catched = true;
    }
    BUG_ON (catched);

    try
    {
        uint8_t payload[(MAX_DOUBLE_TAGGED_PACKET - 30)];
        memset (payload, 0, sizeof (payload));
        cEthernetPacket obj(MAX_DOUBLE_TAGGED_PACKET+1);
        memset (obj.packet, 0xcc, MAX_DOUBLE_TAGGED_PACKET+1);
        obj.reset();
        obj.addVlanTag(false, 12, 7, 0);
        BUG_ON (obj.packet[18] == 0xcc);
        obj.addVlanTag(true, 12, 7, 0);
        BUG_ON (obj.packet[22] == 0xcc);
        obj.addSnapHeader(0x123456, 1234);
        BUG_ON (obj.packet[30] == 0xcc);
        obj.setPayload (payload, sizeof (payload));
        BUG_ON (obj.packet[30] == 0);
        BUG_ON (obj.packet[MAX_DOUBLE_TAGGED_PACKET-1] == 0);
        BUG_ON (obj.packet[MAX_DOUBLE_TAGGED_PACKET] == 0xcc);
    }
    catch (FormatException& )
    {
        BUG_ON (0);
    }
    try
    {
        catched = false;
        uint8_t payload[(MAX_DOUBLE_TAGGED_PACKET - 29)];
        memset (payload, 0, sizeof (payload));
        cEthernetPacket obj(MAX_DOUBLE_TAGGED_PACKET);
        memset (obj.packet, 0xcc, MAX_DOUBLE_TAGGED_PACKET);
        obj.reset();
        obj.addVlanTag(false, 12, 7, 0);
        BUG_ON (obj.packet[18] == 0xcc);
        obj.addVlanTag(true, 12, 7, 0);
        BUG_ON (obj.packet[22] == 0xcc);
        obj.addSnapHeader(0x123456, 1234);
        BUG_ON (obj.packet[30] == 0xcc);
        obj.setPayload (payload, sizeof (payload));
        BUG_ON (obj.packet[30] == 0);
        BUG_ON (obj.packet[MAX_DOUBLE_TAGGED_PACKET-1] == 0);
    }
    catch (FormatException& )
    {
        catched = true;
    }
    BUG_ON (catched);
}
#endif
