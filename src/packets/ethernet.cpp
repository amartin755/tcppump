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
#include <cstddef>

#include "ethernet.hpp"
#include "settings.hpp"
#include "bug.hpp"

#ifdef WITH_UNITTESTS
#include "console.hpp"
#define MEMSET_VAL 0xef
#endif

namespace Protocols
{

Ethernet::Ethernet (std::unique_ptr<Protocol> protocol) : 
    m_protocol (std::move (protocol)),
    m_packetMaxLength (cSettings::get().getMyMTU() + sizeof (mac_header_t) + 4),
    m_allocSize64 ((m_packetMaxLength + sizeof (uint64_t) - 1) / sizeof (uint64_t))
{
    uint64_t* p = new uint64_t[m_allocSize64];
#ifdef WITH_UNITTESTS
    std::memset (p, MEMSET_VAL, m_allocSize64 * sizeof (uint64_t));
#endif
    m_data.emplace_back (p, 0);
    reset ();
}


Ethernet::~Ethernet ()
{
    for (auto& d : m_data)
        delete[] d.first;
}


void Ethernet::reset ()
{
    m_payloadOffset          = sizeof (mac_header_t);
    m_EthertypeLengthOffset  = offsetof (mac_header_t, ethertypeLength);
    m_llcHeaderLength        = 0;
    m_hasDMAC                = false;
    m_hasEthertype           = false;
    *ptrEthertypeLength (0)  = 0;
    setPayloadLength (0, 0);
}


void Ethernet::compile ()
{
    const auto& payload = m_protocol->find (&PAR_ETH_PAYLOAD)->asStream();
    uint8_t* p = compile (payload.size());
    std::memcpy (p, payload.data(), payload.size());
}


uint8_t* Ethernet::compile (size_t payloadLength, const cMacAddress* dstMac)
{
    reset ();

    bool optionalDMAC = dstMac != nullptr;

    // if dstMac is not provided, PAR_ETH_DMAC is mandatory. Otherwise we use dstMac, but only if
    // PAR_ETH_DMAC is not provided!
    // -> PAR_ETH_DMAC has priority over an upper layer provided dstMac
    const cMacAddress& dmac = optionalDMAC 
        ? m_protocol->getValueOrDefault (&PAR_ETH_DMAC, *dstMac) 
        : m_protocol->getValue<cMacAddress> (&PAR_ETH_DMAC);
    const cMacAddress& smac = m_protocol->getValueOrDefault (&PAR_ETH_SMAC, cSettings::get().getMyMAC());
    setMacHeader (smac, dmac);

    // VLAN tags
    ProtocolParameter* parVid = nullptr;
    while ((parVid = m_protocol->findInRange (&PAR_ETH_VID, parVid, nullptr, true)) != nullptr)
    {
        bool isCTag   = m_protocol->getValueInRangeOrDefault (&PAR_ETH_VTYPE, parVid, &PAR_ETH_VID, 1) == 1;
        uint16_t vid  = parVid->asInt16();
        uint16_t prio = m_protocol->getValueInRangeOrDefault (&PAR_ETH_PRIO, parVid, &PAR_ETH_VID, 0);
        uint16_t dei  = m_protocol->getValueInRangeOrDefault (&PAR_ETH_DEI, parVid, &PAR_ETH_VID, 0);
        addVlanTag (isCTag, vid, prio, dei);
    }

    // LLC header
    // NOTE: dsap and ssap are mandatory parameters for LLC header;
    //       if only one of them is defined, we ignore all LLC parameters
    ProtocolParameter* parDsap = m_protocol->find (&PAR_ETH_DSAP, true);
    if (parDsap)
    {
        uint8_t  dsap = parDsap->asInt8();
        uint8_t  ssap = m_protocol->getValue<uint8_t> (&PAR_ETH_SSAP);
        uint16_t ctrl = m_protocol->getValueOrDefault<uint16_t> (&PAR_ETH_CONTROL, 0x0003);
        addLlcHeader (dsap, ssap, ctrl);
    }
    else
    {
        // if there is no dsap parameter, we check for a SNAP header
        ProtocolParameter* parOui = m_protocol->find (&PAR_ETH_OUI, true);
        if (parOui)
        {
            uint32_t oui      = parOui->asInt32();
            uint16_t protocol = m_protocol->getValue<uint16_t> (&PAR_ETH_PROTOCOL);
            addSnapHeader (oui, protocol);
        }
    }

    // reserve space for payload
    setPayloadLength  (0, 0);
    checkPacketLength (0, payloadLength);
    setPayloadLength  (0, payloadLength);

    // if llc header or no ethertype/length is provided, we calculate the length ourself
    ProtocolParameter* parEthertypeLength = nullptr;
    if (hasLlcHeader () || (parEthertypeLength = m_protocol->find (&PAR_ETH_ETHERTYPE, true)) == nullptr)
    {
        setLength (0);
    }
    else
    {
        m_hasEthertype = true;
        setTypeLength (parEthertypeLength->asInt16 ());
    }

    // return pointer to payload
    return ptrPayload (0);
}


uint8_t* Ethernet::compileFragment (size_t fragment, size_t payloadLength)
{
    BUG_ON (fragment < 1); // compile() must be used for the first fragment

    // allocate memory
    if (fragment >= m_data.size ())
    {
        uint64_t* p = new uint64_t[m_allocSize64];
#ifdef WITH_UNITTESTS
        std::memset (p, MEMSET_VAL, m_allocSize64 * sizeof (uint64_t));
#endif
        m_data.emplace_back (p, 0);
    }

    // copy header of first fragment
    std::memcpy (ptrPacket(fragment), ptrPacket(0), m_payloadOffset);

    // reserve space for payload
    setPayloadLength  (fragment, 0);
    checkPacketLength (fragment, payloadLength);
    setPayloadLength  (fragment, payloadLength);

    // adjust length in case of non-ethertype
    if (!m_hasEthertype)
        setLength (fragment);

    // return pointer to payload
    return ptrPayload (fragment);
}


void Ethernet::addLlcHeader (uint8_t dsap, uint8_t ssap, uint16_t control)
{
    // size of the control word can either be 8 or 16 bits (depends on its content)
    m_llcHeaderLength = (control & 0x0003) == 3 ?
            sizeof (llc_t) - 1 : sizeof (llc_t);

    checkPacketLength (0, m_llcHeaderLength);

    llc_t* llc = (llc_t*)ptrPayload (0);

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

    m_payloadOffset += m_llcHeaderLength;
}


void Ethernet::addSnapHeader (uint32_t oui, uint16_t protocol)
{
    addLlcHeader (0xaa, 0xaa, 3);

    checkPacketLength (0, sizeof (snap_t));

    m_llcHeaderLength += sizeof (snap_t);
    oui = htonl (oui);

    snap_t* snap   = (snap_t*)ptrPayload (0);
    snap->oui.a    = uint8_t((oui >>  8) & 0x000000ff);
    snap->oui.b    = uint8_t((oui >> 16) & 0x000000ff);
    snap->oui.c    = uint8_t((oui >> 24) & 0x000000ff);
    snap->protocol = htons (protocol);

    m_payloadOffset += sizeof (snap_t);
}


void Ethernet::addVlanTag (bool isCTag, uint16_t id, uint16_t prio, uint16_t dei)
{
    checkPacketLength (0, sizeof (vlan_t));

    vlan_t* tag = (vlan_t*)ptrEthertypeLength (0);

    isCTag ? tag->setCTag (id, prio, dei) : tag->setSTag (id, prio, dei);

    m_payloadOffset += sizeof (vlan_t);
    m_EthertypeLengthOffset += sizeof (vlan_t);
}

/*
void Ethernet::setPayload (const uint8_t* payload, size_t len)
{
    m_payloadLength = 0;
    checkPacketLength (len);
    std::memcpy (this->ptrPayload (0), payload, len);
    m_payloadLength = len;
}


void Ethernet::appendPayload (const uint8_t* payload, size_t len)
{
    checkPacketLength (len);
    uint8_t* p = this->ptrPayload (0) + m_payloadLength;
    std::memcpy (p, payload, len);
    m_payloadLength += len;
}


void Ethernet::setRaw (const uint8_t* payload, size_t len)
{
    reset ();
    if (unlikely (len > m_packetMaxLength))
        throw FormatException (exParRange, NULL);
    std::memcpy (ptrPacket(0), payload, len);
    m_payloadLength = len - sizeof (mac_header_t);
    m_hasDMAC       = true;
}
*/




#ifdef WITH_UNITTESTS
void Ethernet::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234)")));
        BUG_ON (obj.m_payloadOffset != 14);
        BUG_ON (obj.m_EthertypeLengthOffset != 12);
        BUG_ON (obj.m_llcHeaderLength != 0);
        BUG_ON (obj.m_hasDMAC);
        BUG_ON (obj.m_hasEthertype);
        BUG_ON (obj.m_data.size() != 1);
        BUG_ON (obj.length(0) != 14);
        BUG_ON (obj.payloadLength(0) != 0);
        BUG_ON (!obj.checkConsistency ());
        obj.compile ();
        BUG_ON (obj.m_payloadOffset != 14);
        BUG_ON (obj.m_EthertypeLengthOffset != 12);
        BUG_ON (obj.m_llcHeaderLength != 0);
        BUG_ON (!obj.m_hasDMAC);
        BUG_ON (obj.m_hasEthertype);
        BUG_ON (obj.m_data.size() != 1);
        BUG_ON (obj.length(0) != 16);
        BUG_ON (obj.payloadLength(0) != 2);
        const auto& [data, len] = obj.get(0);
        BUG_ON (len != 16);
        BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x00\x02\x12\x34", len));
        BUG_ON (!obj.checkConsistency ());
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234, ethertype=0x800)")));
        for (int n = 0; n < 2; n++)
        {
            obj.compile ();
            BUG_ON (obj.m_payloadOffset != 14);
            BUG_ON (obj.m_EthertypeLengthOffset != 12);
            BUG_ON (obj.m_llcHeaderLength != 0);
            BUG_ON (!obj.m_hasDMAC);
            BUG_ON (!obj.m_hasEthertype);
            BUG_ON (obj.m_data.size() != 1);
            BUG_ON (obj.length(0) != 16);
            BUG_ON (obj.payloadLength(0) != 2);
            const auto& [data, len] = obj.get(0);
            BUG_ON (len != 16);
            BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x08\x00\x12\x34", len));
            BUG_ON (!obj.checkConsistency ());
        }
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234, ethertype=0x800, vid=0x42)")));
        for (int n = 0; n < 2; n++)
        {
            obj.compile ();
            BUG_ON (obj.m_payloadOffset != 18);
            BUG_ON (obj.m_EthertypeLengthOffset != 16);
            BUG_ON (obj.m_llcHeaderLength != 0);
            BUG_ON (!obj.m_hasDMAC);
            BUG_ON (!obj.m_hasEthertype);
            BUG_ON (obj.m_data.size() != 1);
            BUG_ON (obj.length(0) != 20);
            BUG_ON (obj.payloadLength(0) != 2);
            const auto& [data, len] = obj.get(0);
            BUG_ON (len != 20);
            BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x81\x00\x00\x42\x08\x00\x12\x34", len));
            BUG_ON (!obj.checkConsistency ());
        }
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234, ethertype=0x800, vid=0x1, prio=1, dei=1, vtype=2)")));
        for (int n = 0; n < 2; n++)
        {
            obj.compile ();
            BUG_ON (obj.m_payloadOffset != 18);
            BUG_ON (obj.m_EthertypeLengthOffset != 16);
            BUG_ON (obj.m_llcHeaderLength != 0);
            BUG_ON (!obj.m_hasDMAC);
            BUG_ON (!obj.m_hasEthertype);
            BUG_ON (obj.m_data.size() != 1);
            BUG_ON (obj.length(0) != 20);
            BUG_ON (obj.payloadLength(0) != 2);
            const auto& [data, len] = obj.get(0);
            BUG_ON (len != 20);
            BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x88\xa8\x30\x01\x08\x00\x12\x34", len));
            BUG_ON (!obj.checkConsistency ());
        }
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234, ethertype=0x800, vid=0x1, prio=1, dei=1, vtype=2, vid=42)")));
        for (int n = 0; n < 2; n++)
        {
            obj.compile ();
            BUG_ON (obj.m_payloadOffset != 22);
            BUG_ON (obj.m_EthertypeLengthOffset != 20);
            BUG_ON (obj.m_llcHeaderLength != 0);
            BUG_ON (!obj.m_hasDMAC);
            BUG_ON (!obj.m_hasEthertype);
            BUG_ON (obj.m_data.size() != 1);
            BUG_ON (obj.length(0) != 24);
            BUG_ON (obj.payloadLength(0) != 2);
            const auto& [data, len] = obj.get(0);
            BUG_ON (len != 24);
            BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x88\xa8\x30\x01\x81\x00\x00\x2a\x08\x00\x12\x34", len));
            BUG_ON (!obj.checkConsistency ());
        }
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234, dsap=1, ssap=2, control=3)")));
        for (int n = 0; n < 2; n++)
        {
            obj.compile ();
            BUG_ON (obj.m_payloadOffset != 17);
            BUG_ON (obj.m_EthertypeLengthOffset != 12);
            BUG_ON (obj.m_llcHeaderLength != 3);
            BUG_ON (!obj.m_hasDMAC);
            BUG_ON (obj.m_hasEthertype);
            BUG_ON (obj.m_data.size() != 1);
            BUG_ON (obj.length(0) != 19);
            BUG_ON (obj.payloadLength(0) != 2);
            const auto& [data, len] = obj.get(0);
            BUG_ON (len != 19);
            BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x00\x05\x01\x02\x03\x12\x34", len));
            BUG_ON (!obj.checkConsistency ());
        }
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234, dsap=1, ssap=2, control=4)")));
        for (int n = 0; n < 2; n++)
        {
            obj.compile ();
            BUG_ON (obj.m_payloadOffset != 18);
            BUG_ON (obj.m_EthertypeLengthOffset != 12);
            BUG_ON (obj.m_llcHeaderLength != 4);
            BUG_ON (!obj.m_hasDMAC);
            BUG_ON (obj.m_hasEthertype);
            BUG_ON (obj.m_data.size() != 1);
            BUG_ON (obj.length(0) != 20);
            BUG_ON (obj.payloadLength(0) != 2);
            const auto& [data, len] = obj.get(0);
            BUG_ON (len != 20);
            BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x00\x06\x01\x02\x00\x04\x12\x34", len));
            BUG_ON (!obj.checkConsistency ());
        }
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234, dsap=1, ssap=2, control=4, vid=0x42)")));
        for (int n = 0; n < 2; n++)
        {
            obj.compile ();
            BUG_ON (obj.m_payloadOffset != 22);
            BUG_ON (obj.m_EthertypeLengthOffset != 16);
            BUG_ON (obj.m_llcHeaderLength != 4);
            BUG_ON (!obj.m_hasDMAC);
            BUG_ON (obj.m_hasEthertype);
            BUG_ON (obj.m_data.size() != 1);
            BUG_ON (obj.length(0) != 24);
            BUG_ON (obj.payloadLength(0) != 2);
            const auto& [data, len] = obj.get(0);
            BUG_ON (len != 24);
            BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x81\x00\x00\x42\x00\x06\x01\x02\x00\x04\x12\x34", len));
            BUG_ON (!obj.checkConsistency ());
        }
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }

    try
    {
        Ethernet obj(std::unique_ptr<Protocol>(new Protocol ("eth(dmac=11:22:33:44:55:66, smac=10:20:30:40:50:60, payload=1234, dsap=1, ssap=2, control=4, vid=0x42, vid=4095)")));
        for (int n = 0; n < 2; n++)
        {
            obj.compile ();
            BUG_ON (obj.m_payloadOffset != 26);
            BUG_ON (obj.m_EthertypeLengthOffset != 20);
            BUG_ON (obj.m_llcHeaderLength != 4);
            BUG_ON (!obj.m_hasDMAC);
            BUG_ON (obj.m_hasEthertype);
            BUG_ON (obj.m_data.size() != 1);
            BUG_ON (obj.length(0) != 28);
            BUG_ON (obj.payloadLength(0) != 2);
            const auto& [data, len] = obj.get(0);
            BUG_ON (len != 28);
            BUG_ON (memcmp (data, "\x11\x22\x33\x44\x55\x66\x10\x20\x30\x40\x50\x60\x81\x00\x00\x42\x81\x00\x0f\xff\x00\x06\x01\x02\x00\x04\x12\x34", len));
            BUG_ON (!obj.checkConsistency ());
        }
    }
    catch (...)
    {
        BUG ("expected not to throw");
    }
}

bool Ethernet::checkConsistency () const
{
    for (const auto& d : m_data)
    {
        if (d.first == nullptr)
            return false;
        if (d.second + m_payloadOffset > m_packetMaxLength)
            return false;
        
        uint8_t* p = reinterpret_cast<uint8_t*>(d.first) + m_payloadOffset + d.second;
        while (p < reinterpret_cast<uint8_t*>(d.first) + m_allocSize64 * sizeof (uint64_t))
        {
            if (*p != MEMSET_VAL)
                return false;
            p++;
        }
    }
    return true;
}
#endif

}