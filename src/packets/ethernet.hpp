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


#ifndef ETHERNET_HPP_
#define ETHERNET_HPP_

#include <cstdint>
#include <cstddef>    // size_t, offsetof
#include <memory>
#include <utility>
#include <vector>

#include "bug.hpp"
#include "formatexception.hpp"
#include "inet.h"
#include "macaddress.hpp"
#include "linkable.hpp"
#include "parser.hpp"


namespace Protocols
{

class Ethernet : public cLinkable
{
public:
    Ethernet (const Ethernet&) = delete;
    Ethernet (const Ethernet&&) = delete;
    Ethernet& operator= (const Ethernet&) = delete;
    Ethernet& operator= (const Ethernet&&) = delete;

    explicit Ethernet(std::unique_ptr<Protocol> protocol);
    virtual ~Ethernet ();

    void compile ();
    inline size_t fragments () const {return m_data.size();}
    inline std::pair<const uint8_t*, size_t> get (size_t fragment) const
    {
        assert (fragment < m_data.size());
        return std::make_pair (
            reinterpret_cast<const uint8_t*>(m_data[fragment].first),
            length (fragment));
    }
    inline size_t payloadLength (size_t fragment) const
    {
        assert (fragment < m_data.size());
        return m_data[fragment].second;
    }
    inline size_t length (size_t fragment) const
    {
        return m_payloadOffset + payloadLength (fragment);
    }
/*
    void setPayload (const uint8_t* payload, size_t len);
    void appendPayload (const uint8_t* payload, size_t len);
    void setRaw (const uint8_t* payload, size_t len);
    inline size_t getLength () const {return m_payloadOffset + m_payloadLength;}
    inline bool hasDestMac () const {return m_hasDMAC;}
    inline const uint8_t * getPayload () const {return m_packet + m_payloadOffset;}
    inline size_t getPayloadLength () const {return m_payloadLength;}
    inline uint16_t getTypeLength () const {return ntohs(*ptrEthertypeLength (0));}
*/
    static constexpr size_t   MAX_ETHERNET_PAYLOAD     = 1500;
    static constexpr size_t   MAX_PACKET               = 6+6+2+MAX_ETHERNET_PAYLOAD;
    static constexpr size_t   MAX_TAGGED_PACKET        = MAX_PACKET + 4;
    static constexpr size_t   MAX_DOUBLE_TAGGED_PACKET = MAX_TAGGED_PACKET + 4;

    #pragma pack(push, 1)
    struct mac_header_t
    {
        cMacAddress::mac_t  dest;
        cMacAddress::mac_t  src;
        uint16_t ethertypeLength;
    };
    #pragma pack(pop)

    enum ethertypes_t : uint16_t
    {
        ETHERTYPE_IPV4  = 0x0800,
        ETHERTYPE_ARP   = 0x0806,
        ETHERTYPE_CVLAN = 0x8100,
        ETHERTYPE_IPV6  = 0x86DD,
        ETHERTYPE_SVLAN = 0x88a8,
        ETHERTYPE_PN    = 0x8892,
        ETHERTYPE_LLDP  = 0x88CC
    };

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

protected:
    inline void setTypeLength (uint16_t ethertypeLength)
    {
        *ptrEthertypeLength (0) = htons (ethertypeLength);
    }
    inline void setLength (unsigned fragment)
    {
        setTypeLength (uint16_t(payloadLength(fragment) + m_llcHeaderLength));
    }
    uint8_t* compile (size_t payloadLength, const cMacAddress* dstMac = nullptr);
    uint8_t* compileFragment (size_t fragment, size_t payloadLength);

    std::unique_ptr<Protocol> m_protocol;



private:

    #pragma pack(push, 1)
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
    #pragma pack(pop)

    void reset ();
    inline void checkPacketLength (size_t fragment,  size_t addedBytes) const
    {
        if (unlikely((length (fragment) + addedBytes) > m_packetMaxLength))
            throw FormatException (exParRange, NULL);
    }
    inline bool hasLlcHeader () const {return m_llcHeaderLength != 0;}
    void addLlcHeader (uint8_t dsap, uint8_t ssap, uint16_t control);
    void addSnapHeader (uint32_t oui, uint16_t protocol);
    void addVlanTag (bool isCTag, uint16_t id, uint16_t prio, uint16_t dei);

    inline uint8_t* ptrPacket (unsigned fragment) const
    {
        return reinterpret_cast<uint8_t*>(std::get<0>(m_data[fragment]));
    }
    inline uint8_t* ptrPayload (unsigned fragment) const
    {
        return ptrPacket (fragment) + m_payloadOffset;
    }
    inline uint16_t* ptrEthertypeLength (unsigned fragment) const
    {
        return reinterpret_cast <uint16_t*>(ptrPacket (fragment) + m_EthertypeLengthOffset);
    }
    inline void setPayloadLength (unsigned fragment, size_t paylodLength)
    {
        m_data[fragment].second = paylodLength;
    }
    inline void setMacHeader (const cMacAddress& src, const cMacAddress& dest)
    {
        setSrcMac (src);
        setDestMac (dest);
    }
    inline void setDestMac (const cMacAddress& dest)
    {
        // mac header contains source and destination mac and is always at the begin of the packet
        mac_header_t* header = (mac_header_t*)ptrPacket(0);
        std::memcpy(&header->dest, dest.get(), dest.size());
        m_hasDMAC = true;
    }
    inline void getDestMac (cMacAddress& dest) const
    {
        mac_header_t* header = (mac_header_t*)ptrPacket(0);
        dest.set(&header->dest, sizeof (header->dest));
    }
    inline void setSrcMac (const cMacAddress& src)
    {
        // mac header contains source and destination mac and is always at the begin of the packet
        mac_header_t* header = (mac_header_t*)ptrPacket(0);
        std::memcpy(&header->src, src.get(), src.size());
    }
    inline void getSrcMac (cMacAddress& src) const
    {
        mac_header_t* header = (mac_header_t*)ptrPacket(0);
        src.set(&header->src, sizeof (header->src));
    }
#ifdef WITH_UNITTESTS
    bool checkConsistency () const;
#endif

    /**
     * holds the data of all fragments
     * 
     * first: pointer to the data, where the packet is stored
     * second: size of the payload in bytes
     */
    std::vector <std::pair<uint64_t*, size_t>> m_data;
    const size_t m_packetMaxLength;  // maximum length of the packet in bytes (including mac header and payload)
    const size_t m_allocSize64;

//    const uint32_t* m_data;       // holds the packet data; do never access directly; use packet instead!
    size_t  m_payloadOffset;         // points at begin of payload (will be moved in case of tagging)
    size_t  m_EthertypeLengthOffset; // points at ethertype/length field (will be moved in case of tagging)
    size_t    m_llcHeaderLength;
    bool      m_hasDMAC;
    bool   m_hasEthertype;
};

}

#endif /* ETHERNET_HPP_ */
