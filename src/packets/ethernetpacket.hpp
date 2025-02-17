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


#ifndef ETHERNET_PACKET_H_
#define ETHERNET_PACKET_H_

#include <cstdint>
#include <cstddef>    // size_t

#include "bug.hpp"
#include "formatexception.hpp"
#include "inet.h"
#include "macaddress.hpp"
#include "linkable.hpp"


class cEthernetPacket : public cLinkable
{
public:
    cEthernetPacket ();
    cEthernetPacket (size_t maxLength);
    cEthernetPacket (const cEthernetPacket& obj); // copy constructor
    virtual ~cEthernetPacket ();
    cEthernetPacket (cEthernetPacket&& other);
    cEthernetPacket& operator=(cEthernetPacket&& other);
    void operator=(const cEthernetPacket&) = delete;       // no copy-assignment operator

    void setRandomSrcMac (bool unicast = true, bool multicast = false)
    {
        setSrcMac (cMacAddress (unicast, multicast));
    }
    void setRandomDestMac (bool unicast = true, bool multicast = false)
    {
        setDestMac (cMacAddress (unicast, multicast));
    }
    void setSrcMac (const cMacAddress& src);
    void getSrcMac (cMacAddress& src) const;
    void setDestMac (const cMacAddress& dest);
    void getDestMac (cMacAddress& dest) const;
    void setMacHeader (const cMacAddress& src, const cMacAddress& dest);
    void addLlcHeader (uint8_t dsap, uint8_t ssap, uint16_t control);
    void addSnapHeader (uint32_t oui, uint16_t protocol);
    void addVlanTag (bool isCTag, uint16_t id, uint16_t prio, uint16_t dei);
    void setTypeLength (uint16_t ethertypeLenth);
    void setLength ();
    void setPayload (const uint8_t* payload, size_t len);
    void appendPayload (const uint8_t* payload, size_t len);
    void setRaw (const uint8_t* payload, size_t len);
    const uint8_t* get () const;
    inline size_t getLength () const {return pPayload - packet + payloadLength;}
    inline void clear () {reset ();};
    inline bool hasLlcHeader () const {return llcHeaderLength != 0;}
    inline bool hasPayload () const {return payloadLength != 0;}
    inline bool hasDestMac () const {return hasDMAC;}
    inline const uint8_t * getPayload () const {return pPayload;}
    inline uint8_t getPayloadAt8 (unsigned offset) const
    {
        if (offset > payloadLength)
            throw FormatException (exParRange, NULL);

        return pPayload[offset];
    }
    inline uint16_t getPayloadAt16 (unsigned offset) const // note: offset is a byte offset!!!
    {
        if (offset > payloadLength)
            throw FormatException (exParRange, NULL);

        return ((uint16_t*)pPayload)[offset/2];
    }
    inline size_t getPayloadLength () const {return payloadLength;}
    inline uint16_t getTypeLength () const {return ntohs(*pEthertypeLength);}
    void updatePayloadAt (unsigned offset, const void* payload, size_t len);

    static const size_t   MAX_ETHERNET_PAYLOAD     = 1500;
    static const size_t   MAX_PACKET               = 6+6+2+MAX_ETHERNET_PAYLOAD;
    static const size_t   MAX_TAGGED_PACKET        = MAX_PACKET + 4;
    static const size_t   MAX_DOUBLE_TAGGED_PACKET = MAX_TAGGED_PACKET + 4;

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

protected:
    inline size_t getMtu () const {return pPayload - packet + packetMaxLength;}

private:
    void reset ();
    void updatePosition (size_t len);
    inline void checkPacketLength (size_t addedBytes) const
    {
        if ((getLength () + addedBytes) > packetMaxLength)
            throw FormatException (exParRange, NULL);
    }

    const uint32_t* data;       // holds the packet data; do never access directly; use packet instead!
    uint8_t*  packet;            // always points to packet begin
    size_t    packetMaxLength;
    uint8_t*  pPayload;         // points at begin of payload (will be moved in case of tagging)
    uint16_t* pEthertypeLength; // points at ethertype/length field (will be moved in case of tagging)
    size_t    payloadLength;
    size_t    llcHeaderLength;
    bool      hasDMAC;
};

enum ethertypes_t : uint16_t
{
    ETHERTYPE_IPV4  = 0x0800,
    ETHERTYPE_ARP   = 0x0806,
    ETHERTYPE_CVLAN = 0x8100,
    ETHERTYPE_IPV6  = 0x86DD,
    ETHERTYPE_SVLAN = 0x88a8,
    ETHERTYPE_PN    = 0x8892,
};

#pragma pack(1)

struct mac_header_t
{
    cMacAddress::mac_t  dest;
    cMacAddress::mac_t  src;
    uint16_t ethertypeLength;
};

struct vlan_t
{
    uint16_t tpid;
    uint16_t tci;  // tag control information | prio (3 bit) | CFI/DEI (1 bit) | vlan id (12 bit)

public:
    void setCTag (uint16_t id, uint16_t prio = 0, uint16_t dei = 0)
    {
        tpid = htons (ETHERTYPE_CVLAN);
        setTci (id, prio, dei);
    }
    void setSTag (uint16_t id, uint16_t prio = 0, uint16_t dei = 0)
    {
        tpid = htons (ETHERTYPE_SVLAN);
        setTci (id, prio, dei);
    }
    unsigned getId () const
    {
        return unsigned (ntohs (tci) & 0x03ff);
    }
    unsigned getPrio () const
    {
        return unsigned ((ntohs (tci) >> 13) & 0x0007);
    }
    unsigned getDEI () const
    {
        return unsigned ((ntohs (tci) >> 12) & 0x0001);
    }
    bool isVlan () const
    {
        uint16_t type = ntohs (tpid);
        return type == ETHERTYPE_CVLAN || type == ETHERTYPE_SVLAN;
    }
    bool isCVlan () const
    {
        return ntohs (tpid) == ETHERTYPE_CVLAN;
    }
    bool isPVlan () const
    {
        return ntohs (tpid) == ETHERTYPE_SVLAN;
    }


private:
    void setTci (uint16_t vid, uint16_t prio, uint16_t dei)
    {
        tci = htons ((vid & 0x0FFF)| ((dei & 1) << 12) | ((prio & 7) << 13));
    }

};

struct llc_t
{
    uint8_t  dsap;
    uint8_t  ssap;
    union
    {
        uint16_t c16;
        uint8_t  c8;
    }control;
};

struct oui_t
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
};

struct snap_t
{
    oui_t    oui;
    uint16_t protocol;
};

#pragma pack()

#endif /* ETHERNET_PACKET_H_ */
