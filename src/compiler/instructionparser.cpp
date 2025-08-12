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


#include <cstdio>
#include <cctype>
#include <cstring>

#include "instructionparser.hpp"

#include "bug.hpp"
#include "parsehelper.hpp"
#include "parameterlist.hpp"
#include "ethernetpacket.hpp"
#include "arppacket.hpp"
#include "ippacket.hpp"
#include "udppacket.hpp"
#include "vxlanpacket.hpp"
#include "tcppacket.hpp"
#include "vrrppacket.hpp"
#include "stppacket.hpp"
#include "igmppacket.hpp"
#include "icmppacket.hpp"
#include "grepacket.hpp"
#include "lldppacket.hpp"
#include "settings.hpp"
#include "endian.h"
#include "bytearray.hpp"
#include "syntax.hpp"
#include "console.hpp"
#include "lldpparser.hpp"


cInstructionParser::cInstructionParser (bool optDestMAC)
: m_currentInstruction (nullptr), m_ipOptionalDestMAC (optDestMAC),  m_recursionDepth (0)
{
}


cInstructionParser::~cInstructionParser ()
{
}

void cInstructionParser::parse (const char* instruction, cResult& result, bool ignoreTrailingGarbage, bool noEthHeader)
{
    const char* prevInstruction = m_currentInstruction; // save in case of recursion
    m_currentInstruction = instruction;

    const char* p = instruction;
    const char* keyword;
    size_t      keywordLen;


    p = parseTimestamp (p, result.hasTimestamp, result.timestamp, result.isAbsolute);
    p = parseProtocollIdentifier (p, &keyword, &keywordLen);

    // parse protocol parameter list
    cParameterList params (p, ignoreTrailingGarbage);
    if (!params.isValid ())
    {
        throwParseException ("Syntax error", params.getParseError ());
    }

    // compile frames
    try
    {
        // avoid unnecessary recursion (real usecases do not exceed a depth of 2-3)
        if (++m_recursionDepth > 8)
            throwParseException ("Maximum depth of embedded instructions reached", keyword, keywordLen);

        //TODO find better way for protocol selection (e.g. hash table)
        if (!strncmp (PR_RAW.syntax, keyword, keywordLen))
            result.packets = compileRAW (noEthHeader, params);
        else if (!strncmp (PR_ETH.syntax, keyword, keywordLen))
            result.packets = compileETH (params);
        else if (!strncmp (PR_ARP.syntax, keyword, keywordLen))
            result.packets = compileARP (params);
        else if (!strncmp (PR_ARP_PROBE.syntax, keyword, keywordLen))
            result.packets = compileARP (params, true);
        else if (!strncmp (PR_ARP_ANNOUNCE.syntax, keyword, keywordLen))
            result.packets = compileARP (params, false, true);
        else if (!strncmp (PR_IPV4.syntax, keyword, keywordLen))
            result.packets = compileIP (noEthHeader, params, false);
        else if (!strncmp (PR_IPV6.syntax, keyword, keywordLen))
            result.packets = compileIP (noEthHeader, params, true);
        else if (!strncmp (PR_UDP4.syntax, keyword, keywordLen))
            result.packets = compileUDP (noEthHeader, params, false);
        else if (!strncmp (PR_UDP6.syntax, keyword, keywordLen))
            result.packets = compileUDP (noEthHeader, params, true);
        else if (!strncmp (PR_VRRP.syntax, keyword, keywordLen))
            result.packets = compileVRRP (noEthHeader, params, 2);
        else if (!strncmp (PR_VRRP3.syntax, keyword, keywordLen))
            result.packets = compileVRRP (noEthHeader, params, 3);
        else if (!strncmp (PR_STP.syntax, keyword, keywordLen))
            result.packets = compileSTP (noEthHeader, params);
        else if (!strncmp (PR_STP_TCN.syntax, keyword, keywordLen))
            result.packets = compileSTP (noEthHeader, params, false, true);
        else if (!strncmp (PR_RSTP.syntax, keyword, keywordLen))
            result.packets = compileSTP (noEthHeader, params, true);
        else if (!strncmp (PR_IGMP.syntax, keyword, keywordLen))
            result.packets = compileIGMP (noEthHeader, params, false, false, false, false);
        else if (!strncmp (PR_IGMP_QUERY.syntax, keyword, keywordLen))
            result.packets = compileIGMP (noEthHeader, params, false, true, false, false);
        else if (!strncmp (PR_IGMP3_QUERY.syntax, keyword, keywordLen))
            result.packets = compileIGMP (noEthHeader, params, true, true, false, false);
        else if (!strncmp (PR_IGMP_REPORT.syntax, keyword, keywordLen))
            result.packets = compileIGMP (noEthHeader, params, false, false, true, false);
        else if (!strncmp (PR_IGMP_LEAVE.syntax, keyword, keywordLen))
            result.packets = compileIGMP (noEthHeader, params, false, false, false, true);
        else if (!strncmp (PR_ICMP4.syntax, keyword, keywordLen))
            result.packets = compileICMP (noEthHeader, params);
        else if (!strncmp (PR_ICMP4_UNREACH.syntax, keyword, keywordLen))
            result.packets = compileICMPWithEmbedded (noEthHeader, params, 3);
        else if (!strncmp (PR_ICMP4_SRCQ.syntax, keyword, keywordLen))
            result.packets = compileICMPWithEmbedded (noEthHeader, params, 4);
        else if (!strncmp (PR_ICMP4_TIMEX.syntax, keyword, keywordLen))
            result.packets = compileICMPWithEmbedded (noEthHeader, params, 11);
        else if (!strncmp (PR_ICMP4_REDIR.syntax, keyword, keywordLen))
            result.packets = compileICMPRedirect (noEthHeader, params);
        else if (!strncmp (PR_ICMP4_ECHO.syntax, keyword, keywordLen))
            result.packets = compileICMPPing (noEthHeader, params, false);
        else if (!strncmp (PR_ICMP4_ECHOR.syntax, keyword, keywordLen))
            result.packets = compileICMPPing (noEthHeader, params, true);
        else if (!strncmp (PR_TCP4.syntax, keyword, keywordLen))
            result.packets = compileTCP (noEthHeader, params);
        else if (!strncmp (PR_VXLAN4.syntax, keyword, keywordLen))
            result.packets = compileVXLAN (noEthHeader, params, false);
        else if (!strncmp (PR_VXLAN6.syntax, keyword, keywordLen))
            result.packets = compileVXLAN (noEthHeader, params, true);
        else if (!strncmp (PR_GRE4.syntax, keyword, keywordLen))
            result.packets = compileGRE (noEthHeader, params, false);
        else if (!strncmp (PR_GRE6.syntax, keyword, keywordLen))
            result.packets = compileGRE (noEthHeader, params, true);
        else if (!strncmp (PR_LLDP.syntax, keyword, keywordLen))
            result.packets = compileLLDP (noEthHeader, params);
        else
            throwParseException ("Unknown protocol type", keyword, keywordLen);

        params.checkForUnusedParameters ();
        m_currentInstruction = prevInstruction;
        m_recursionDepth--;
        return;
    }
    catch (FormatException& e)
    {
        m_recursionDepth--;

        // in case of errors possibly created packets are freed again
        if (result.packets)
        {
            delete result.packets;
            result.packets = nullptr;
        }

        switch (e.what ())
        {
        case exParUnknown:
            throwParseException ("Missing parameter", p, std::strlen (p), e.value ());
            break;
        case exParRange:
            throwParseException ("Range of parameter violated", e.value (), e.valueLength ());
            break;
        case exParFormat:
            throwParseException ("Invalid parameter value", e.value (), e.valueLength ());
            break;
        case exParUnused:
            throwParseException (e.why(), e.value (), e.valueLength ());
            break;
        default:
            BUG ("BUG: unexpected compile exception");
        }
    }

    BUG ("BUG: unreachable code");
}

const char* cInstructionParser::parseTimestamp (const char* p, bool& hasTimestamp, uint64_t& timestamp, bool& isAbsolute)
{
    hasTimestamp = false;

    // ignore whitespaces at begin of instruction
    p = cParseHelper::skipWhitespaces(p);

    // if first character is a number we assume there is a timestamp
    if (isdigit (*p) || *p == '+')
    {
        char* end;

        // relative timestamp?
        isAbsolute = *p == '+' ? false : true;

        timestamp = ((uint64_t)strtoull (p, &end, 10));

        if (p != end)
        {
            // timestamp must be terminated with ':'
            p = cParseHelper::nextCharIgnoreWhitspaces (end, ':');
            if (!p)
                throwParseException ("Expected ':' after timestamp", end);
            p++;
        }
        else
        {
            throwParseException ("Invalid timestamp", p);
        }
        hasTimestamp = true;
    }

    return p;
}

const char* cInstructionParser::parseProtocollIdentifier (const char* p, const char** identifier, size_t *len)
{
    const char* keyword = nullptr;
    const char* keywordEnd = nullptr;

    // find beginning of protocol keyword
    p = keyword = cParseHelper::nextKeyStart (p);
    // find end of protocol keyword
    if (keyword)
        p = keywordEnd = cParseHelper::nextKeyEnd (p);
    if (!keyword && !keywordEnd)
        throwParseException ("Missing protocol specifier", p);

    // find begin of parameter list --> '('
    p = cParseHelper::nextCharIgnoreWhitspaces (p, '(');
    if (!p)
        throwParseException ("Expected '(' after protocol specifier", keyword);



    *identifier = keyword;
    *len = keywordEnd - keyword;

    return p;
}


cLinkable* cInstructionParser::compileRAW (bool noEthHeader, cParameterList& params)
{
    cByteArray payload;

    for (auto &par : params)
    {
        auto name = par.name();
        params.setParameterUsed (&par, true);
        if (!strncmp (PAR_RAW_BYTE.syntax, name.first, name.second))
        {
            payload << par.asInt8 ();
        }
        else if (!strncmp (PAR_RAW_BE16.syntax, name.first, name.second))
        {
            payload << toBE16 (par.asInt16 ());
        }
        else if (!strncmp (PAR_RAW_BE32.syntax, name.first, name.second))
        {
            payload << toBE32 (par.asInt32 ());
        }
        else if (!strncmp (PAR_RAW_BE64.syntax, name.first, name.second))
        {
            payload << toBE64 (par.asInt64 ());
        }
        else if (!strncmp (PAR_RAW_LE16.syntax, name.first, name.second))
        {
            payload << toLE16 (par.asInt16 ());
        }
        else if (!strncmp (PAR_RAW_LE32.syntax, name.first, name.second))
        {
            payload << toLE32 (par.asInt32 ());
        }
        else if (!strncmp (PAR_RAW_LE64.syntax, name.first, name.second))
        {
            payload << toLE64 (par.asInt64 ());
        }
        else if (!strncmp (PAR_RAW_IP4.syntax, name.first, name.second))
        {
            cIPv4 ip4 = par.asIPv4 ();
            payload.append (ip4.getAsArray(), 4);
        }
        else if (!strncmp (PAR_RAW_IP6.syntax, name.first, name.second))
        {
            cIPv6 ip6 = par.asIPv6 ();
            payload.append (ip6.getAsArray(), 16);
        }
        else if (!strncmp (PAR_RAW_MAC.syntax, name.first, name.second))
        {
            cMacAddress mac = par.asMac ();
            payload.append (mac.get (), mac.size ());
        }
        // TODO do we want to allow embedded packets? If yes with or without Ethernet header?
        else if (!strncmp (PAR_RAW_STREAM.syntax, name.first, name.second))
        {
            size_t len;
            const uint8_t* value = par.asStream(len);
//            payload = compileEmbedded (optionalPar, true, len);
            payload.append (value, len);
        }
        else
        {
            params.setParameterUsed (&par, false);
        }
    }

    cEthernetPacket* eth = nullptr;
    try
    {
        eth = new cEthernetPacket (payload.size() + sizeof (mac_header_t));
        if (noEthHeader)
            eth->setPayload (payload.data(), payload.size());
        else
            eth->setRaw (payload.data(), payload.size());
    }
    catch (...)
    {
        delete eth;
        eth = nullptr;
        throw;
    }

    return eth;
}


// returns true, if destination MAC address is set
bool cInstructionParser::compileMacHeader (cParameterList& params, cEthernetPacket *packet, bool noDestination, bool destIsOptional)
{
    // default value of source mac is our own mac address
    packet->setSrcMac (getParameterOrOwnMac (params, PAR_ETH_SMAC.syntax));

    if (!noDestination)
    {
        const cParameter* destMacPar = params.findParameter (PAR_ETH_DMAC.syntax, destIsOptional);
        if (destMacPar)
        {
            packet->setDestMac (destMacPar->asMac ());
            return true;
        }
    }
    return false;
}


cMacAddress cInstructionParser::getParameterOrOwnMac (cParameterList& params, const char* par) const
{
    const cParameter* optionalPar = params.findParameter (par, true);

    return optionalPar ? optionalPar->asMac () : cSettings::get().getMyMAC();
}


cIPv4 cInstructionParser::getParameterOrOwnIPv4 (cParameterList& params, const char* par) const
{
    const cParameter* optionalPar = params.findParameter (par, true);

    return optionalPar ? optionalPar->asIPv4() : cSettings::get().getMyIPv4();
}


cIPv6 cInstructionParser::getParameterOrOwnIPv6 (cParameterList& params, const char* par) const
{
    const cParameter* optionalPar = params.findParameter (par, true);

    return optionalPar ? optionalPar->asIPv6() : cSettings::get().getMyIPv6();
}


const uint8_t* cInstructionParser::compileEmbedded (cParameter* emb, bool noEthHeader, size_t& len)
{
    bool isEmbedded = false;
    const uint8_t* payload = emb->asEmbedded (isEmbedded, len);

    if (isEmbedded)
    {
        cResult res;

        // save current mtu and increase it as we don't want to limit the size of embedded packets.
        // 32K is a arbitrary limit, just to avoid generation of pakets that are impossible to send
        unsigned mtu = cSettings::get().getMyMTU();
        cSettings::get().setMyMTU (32*1024);

        try
        {
            parse ((char*)payload, res, true, noEthHeader);

            // restore changed mtu
            cSettings::get().setMyMTU (mtu);
        }
        catch (...)
        {
            // restore changed mtu
            cSettings::get().setMyMTU (mtu);
            throw;
        }


        cEthernetPacket* eth = dynamic_cast<cEthernetPacket*>(res.packets);
        if (!eth)
        {
            cIPPacket* ipv4 = dynamic_cast<cIPPacket*>(res.packets);
            if (ipv4)
            {
                eth = &ipv4->getFirstEthernetPacket();
            }
        }

        if (eth)
        {
            if (!noEthHeader)
            {
                payload = eth->get ();
                len     = eth->getLength ();
            }
            else
            {
                payload = eth->getPayload ();
                len     = eth->getPayloadLength ();
            }
        }
        else
        {
            throwParseException ("unsupported embedded packet", (char*)payload, len);
        }
    }
    return payload;
}


cLinkable* cInstructionParser::compileETH (cParameterList& params)
{
    cEthernetPacket* eth = new cEthernetPacket;
    const cParameter* optionalPar = nullptr;

    try
    {
        // MAC header
        compileMacHeader (params, eth, false);

        // compile VLAN tags
        compileVLANTags (params, eth);

        // LLC header
        // NOTE: dsap and ssap are mandatory parameters for llc header;
        //       if only one of them is defined, we ignore all LLC parameters
        optionalPar = params.findParameter (PAR_ETH_DSAP.syntax,  true);
        if (optionalPar)
        {
            uint8_t dsap = optionalPar->asInt8 ();
            uint8_t ssap = params.findParameter (PAR_ETH_SSAP.syntax)->asInt8 ();
            eth->addLlcHeader(dsap, ssap, params.findParameter(PAR_ETH_CONTROL.syntax, (uint32_t)3)->asInt16 ());
        }
        else
        {
            // SNAP extension
            optionalPar = params.findParameter (PAR_ETH_OUI.syntax,  true);
            if (optionalPar)
            {
                eth->addSnapHeader (optionalPar->asInt32 (0, 0x00ffffff),
                        params.findParameter (PAR_ETH_PROTOCOL.syntax)->asInt16 ());
            }
        }

        size_t len;
        const uint8_t* value = params.findParameter (PAR_ETH_PAYLOAD.syntax)->asStream(len);
        eth->setPayload (value, len);

        // if llc header or no ethertype/length is provided, we calculate the length ourself
        if (eth->hasLlcHeader() || (optionalPar = params.findParameter (PAR_ETH_ETHERTYPE.syntax, true)) == NULL)
        {
            eth->setLength ();
        }
        else
        {
            eth->setTypeLength (optionalPar->asInt16 ());
        }
    }
    catch (...)
    {
        delete eth;
        eth = nullptr;
        throw;
    }

    return eth;
}


size_t cInstructionParser::compileVLANTags (cParameterList& params, cEthernetPacket *packet)
{
    const cParameter* optionalPar = nullptr;

    // VLAN tags
    while ((optionalPar = params.findParameter(optionalPar, nullptr, PAR_ETH_VID.syntax, true)) != nullptr)
    {
        bool isCTag   = params.findParameter (optionalPar, PAR_ETH_VID.syntax, PAR_ETH_VTYPE.syntax, (uint32_t)1)->asInt8 (1, 2) == 1 ? true : false;
        uint16_t vid  = optionalPar->asInt16 (0, 0x0fff);
        uint16_t prio = params.findParameter (optionalPar, PAR_ETH_VID.syntax, PAR_ETH_PRIO.syntax,  (uint32_t)0)->asInt8 (0, 7);
        uint16_t dei  = params.findParameter (optionalPar, PAR_ETH_VID.syntax, PAR_ETH_DEI.syntax,   (uint32_t)0)->asInt8 (0, 1);
        packet->addVlanTag (isCTag, vid, prio, dei);
    }
    return packet->getLength();
}


cLinkable* cInstructionParser::compileARP (cParameterList& params, bool isProbe, bool isGratuitous)
{
    cArpPacket*  arp = new cArpPacket;
    try
    {

        if (isProbe)
        {
            arp->probe (cSettings::get().getMyMAC(), params.findParameter (PAR_IP_DIP.syntax)->asIPv4());
        }
        else if (isGratuitous)
        {
            arp->announce (cSettings::get().getMyMAC(), getParameterOrOwnIPv4 (params, PAR_IP_DIP.syntax));
        }
        else
        {
            cMacAddress targetMac = params.findParameter (PAR_ETH_DMAC.syntax, cMacAddress ((unsigned)0))->asMac();

            arp->setAll (params.findParameter (PAR_ARP_OP.syntax, (uint32_t)1)->asInt16(),
                         getParameterOrOwnMac (params, PAR_ETH_SMAC.syntax),
                         getParameterOrOwnIPv4 (params, PAR_IP_SIP.syntax),
                         targetMac,
                         params.findParameter (PAR_IP_DIP.syntax)->asIPv4()
                        );
        }

        // compile VLAN tags
        compileVLANTags (params, arp);
    }
    catch (...)
    {
        delete arp;
        arp = nullptr;
        throw;
    }

    return arp;
}


// returns true, if destination IP address is a multicast address
bool cInstructionParser::parseIPv4Params (cParameterList& params, cIPPacket* packet, bool noDestinationIP)
{
    bool isMulticast = false;

    packet->setDSCP         (params.findParameter (PAR_IP_DSCP.syntax, (uint32_t)0)->asInt8(0, 0x3f));
    packet->setECN          (params.findParameter (PAR_IP_ECN.syntax, (uint32_t)0)->asInt8(0, 3));
    packet->setTimeToLive   (params.findParameter (PAR_IP_TTL.syntax, (uint32_t)64)->asInt8());
    packet->setDontFragment (params.findParameter (PAR_IP4_DF.syntax, (uint32_t)0)->asInt8(0, 1));
    if (!noDestinationIP)
    {
        const cIPv4 destIP = params.findParameter (PAR_IP_DIP.syntax)->asIPv4();
        packet->setDestination (destIP);
        isMulticast = destIP.isMulticast();
    }
    packet->setSource (getParameterOrOwnIPv4 (params, PAR_IP_SIP.syntax));
    cParameter* optionalPar = params.findParameter (PAR_IP4_ID.syntax, true);
    if (optionalPar)
        packet->setIdentification(optionalPar->asInt16());

    return isMulticast;
}


// returns true, if destination IP address is a multicast address
bool cInstructionParser::parseIPv6Params (cParameterList& params, cIPPacket* packet, bool noDestinationIP)
{
    bool isMulticast = false;

    packet->setDSCP       (params.findParameter (PAR_IP_DSCP.syntax, (uint32_t)0)->asInt8(0, 0x3f));
    packet->setECN        (params.findParameter (PAR_IP_ECN.syntax,  (uint32_t)0)->asInt8(0, 3));
    packet->setTimeToLive (params.findParameter (PAR_IP_TTL.syntax, (uint32_t)64)->asInt8());

    if (!noDestinationIP)
    {
        const cIPv6 destIP = params.findParameter (PAR_IP_DIP.syntax)->asIPv6();
        packet->setDestination (destIP);
        isMulticast = destIP.isMulticast();
    }
    packet->setSource (getParameterOrOwnIPv6 (params, PAR_IP_SIP.syntax));
    cParameter* optionalPar = params.findParameter (PAR_IP6_FL.syntax, true);
    if (optionalPar)
        packet->setFlowLabel (optionalPar->asInt32(0, 0xfffff));

    return isMulticast;
}


cLinkable* cInstructionParser::compileIP (bool noEthHeader, cParameterList& params, bool isIPv6)
{
    cIPPacket* ippacket = new cIPPacket(isIPv6);
    try
    {
        cEthernetPacket& eth = ippacket->getFirstEthernetPacket();
        bool destIsMulticast = isIPv6 ? parseIPv6Params (params, ippacket) : parseIPv4Params (params, ippacket);

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader  (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags   (params, &eth);
        }
        size_t len;
        const uint8_t* payload = params.findParameter (PAR_IP_PAYLOAD.syntax)->asStream(len);
        ippacket->compile (params.findParameter (PAR_IP_PROTOCOL.syntax)->asInt8(), nullptr, 0, payload, len);
    }
    catch (...)
    {
        delete ippacket;
        ippacket = nullptr;
        throw;
    }
    return ippacket;
}


cLinkable* cInstructionParser::compileUDP (bool noEthHeader, cParameterList& params, bool isIPv6)
{
    cUdpPacket* udppacket = new cUdpPacket (isIPv6);
    try
    {
        cEthernetPacket& eth = udppacket->getFirstEthernetPacket();
        bool destIsMulticast = isIPv6 ? parseIPv6Params (params, udppacket) : parseIPv4Params (params, udppacket);

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader  (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags   (params, &eth);
        }
        udppacket->setSourcePort(params.findParameter (PAR_UDP_SPORT.syntax)->asInt16());
        udppacket->setDestinationPort(params.findParameter (PAR_UDP_DPORT.syntax)->asInt16());

        size_t len = 0;
        const uint8_t* payload = nullptr;
        cParameter* optionalPar = params.findParameter (PAR_UDP_PAYLOAD.syntax, true);
        if (optionalPar)
            payload = optionalPar->asStream(len);
        udppacket->compile (payload, len);

        optionalPar = params.findParameter (PAR_UDP_CHKSUM.syntax, true);
        if (optionalPar)
            udppacket->setChecksum (optionalPar->asInt16());
    }
    catch (...)
    {
        delete udppacket;
        udppacket = nullptr;
        throw;
    }

    return udppacket;
}


cLinkable* cInstructionParser::compileVXLAN (bool noEthHeader, cParameterList& params, bool isIPv6)
{
    cVxlanPacket* vxlanpacket = new cVxlanPacket (isIPv6);
    try
    {
        cEthernetPacket& eth = vxlanpacket->getFirstEthernetPacket();
        bool destIsMulticast = isIPv6 ? parseIPv6Params (params, vxlanpacket) : parseIPv4Params (params, vxlanpacket);

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags  (params, &eth);
        }
        vxlanpacket->setSourcePort (params.findParameter (PAR_UDP_SPORT.syntax)->asInt16());
        vxlanpacket->setDestinationPort (params.findParameter (PAR_UDP_DPORT.syntax, (uint32_t)4789)->asInt16());
        vxlanpacket->setVni (params.findParameter (PAR_VXLAN_VNI.syntax, (uint32_t)0)->asInt32(0, 0x00ffffff));

        size_t len = 0;
        const uint8_t* payload = nullptr;

        cParameter* optionalPar = params.findParameter (PAR_VXLAN_PAYLOAD.syntax, true);
        if (optionalPar)
        {
            payload = compileEmbedded (optionalPar, false, len);
        }
        vxlanpacket->compile (payload, len);
    }
    catch (...)
    {
        delete vxlanpacket;
        vxlanpacket = nullptr;
        throw;
    }

    return vxlanpacket;
}


cLinkable* cInstructionParser::compileTCP (bool noEthHeader, cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);
        bool userDefinedChecksum = false;
        cParameter* optionalPar;

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader  (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags   (params, &eth);
        }
        tcppacket->setSourcePort(params.findParameter (PAR_TCP_SPORT.syntax)->asInt16());
        tcppacket->setDestinationPort(params.findParameter (PAR_TCP_DPORT.syntax)->asInt16());

        tcppacket->setSeqNumber (params.findParameter (PAR_TCP_SEQ.syntax)->asInt32());
        tcppacket->setAckNumber (params.findParameter (PAR_TCP_ACK.syntax)->asInt32());

        tcppacket->setWindow (params.findParameter (PAR_TCP_WIN.syntax, (uint32_t)1024)->asInt16());
        tcppacket->setUrgentPointer (params.findParameter (PAR_TCP_URGPTR.syntax, (uint32_t)0)->asInt16());

        // Flags
        tcppacket->setFlagFIN (!!(params.findParameter (PAR_TCP_FIN.syntax,    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagSYN (!!(params.findParameter (PAR_TCP_SYN.syntax,    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagRST (!!(params.findParameter (PAR_TCP_RESET.syntax,  (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagPSH (!!(params.findParameter (PAR_TCP_PUSH.syntax,   (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagACK (!!(params.findParameter (PAR_TCP_ACKFLAG.syntax,(uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagURG (!!(params.findParameter (PAR_TCP_URGENT.syntax, (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagECE (!!(params.findParameter (PAR_TCP_ECN.syntax,    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagCWR (!!(params.findParameter (PAR_TCP_CWR.syntax,    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagNON (!!(params.findParameter (PAR_TCP_NONCE.syntax,  (uint32_t)0)->asInt8(0, 1)));

        size_t len = 0;
        const uint8_t* payload = nullptr;
        optionalPar = params.findParameter (PAR_TCP_PAYLOAD.syntax, true);
        if (optionalPar)
            payload = optionalPar->asStream(len);

        optionalPar = params.findParameter (PAR_TCP_CHKSUM.syntax, true);
        if (optionalPar)
        {
            userDefinedChecksum = true;
            tcppacket->setChecksum (optionalPar->asInt16());
        }

        tcppacket->compile (payload, len, !userDefinedChecksum);
    }
    catch (...)
    {
        delete tcppacket;
        tcppacket = nullptr;
        throw;
    }

    return tcppacket;
}


cLinkable* cInstructionParser::compileVRRP (bool noEthHeader, cParameterList& params, int version)
{
    cVrrpPacket* vrrp = new cVrrpPacket;
    try
    {
        bool userDefinedChecksum = false;
        cEthernetPacket& eth = vrrp->getFirstEthernetPacket();

        parseIPv4Params   (params, vrrp, true);
        if (!noEthHeader)
        {
            compileMacHeader  (params, &eth, true);
            compileVLANTags   (params, &eth);
        }
        const cParameter* firstVRIP = params.findParameter (PAR_VRRP_VRIP.syntax);

        vrrp->setVersion(version);
        vrrp->setVRID(params.findParameter (PAR_VRRP_VRID.syntax)->asInt8(1, 255));
        vrrp->addVirtualIP(firstVRIP->asIPv4());
        vrrp->setPrio(params.findParameter (PAR_VRRP_VRPRIO.syntax, (uint32_t)100)->asInt8());
        vrrp->setType(params.findParameter (PAR_VRRP_TYPE.syntax, (uint32_t)1)->asInt8(0, 15));
        if (version == 2)
            vrrp->setInterval(params.findParameter (PAR_VRRP_AINT.syntax, (uint32_t)1)->asInt8());
        else
            vrrp->setInterval(params.findParameter (PAR_VRRP_AINT.syntax, (uint32_t)100)->asInt16(0, 4095));
        const cParameter* optionalPar = params.findParameter (PAR_VRRP_CHKSUM.syntax, true);
        if (optionalPar)
        {
            userDefinedChecksum = true;
            vrrp->setChecksum (optionalPar->asInt16());
        }

        // additional virtual IPs (optional)
        optionalPar = firstVRIP;
        int vripCount = 1;
        while ((++vripCount <= 255 ) &&
               ((optionalPar = params.findParameter(optionalPar, nullptr, PAR_VRRP_VRIP.syntax, true)) != nullptr))
        {
            vrrp->addVirtualIP (optionalPar->asIPv4());
        }

        vrrp->compile (!userDefinedChecksum);
    }
    catch (...)
    {
        delete vrrp;
        vrrp = nullptr;
        throw;
    }

    return vrrp;
}


cLinkable* cInstructionParser::compileSTP (bool noEthHeader, cParameterList& params, bool isRSTP, bool isTCN)
{
    cStpPacket*  stp = new cStpPacket;

    try
    {
        if (!noEthHeader)
        {
            compileMacHeader (params, stp, true);
            compileVLANTags  (params, stp);
        }
        if (!isTCN)
        {
            int flags = 0;
            uint32_t pathCost;
            unsigned rootBridgePrio          = params.findParameter (PAR_STP_RBPRIO.syntax, (uint32_t)8)->asInt8 (0, 15);
            unsigned rootBridgeId            = params.findParameter (PAR_STP_RBIDEXT.syntax, (uint32_t)0)->asInt16 (0, 4095);
            const cMacAddress& rootBridgeMac = getParameterOrOwnMac (params, PAR_STP_RBMAC.syntax);
            unsigned bridgePrio              = params.findParameter (PAR_STP_BPRIO.syntax, (uint32_t)8)->asInt8 (0, 15);
            unsigned bridgeId                = params.findParameter (PAR_STP_BIDEXT.syntax, (uint32_t)0)->asInt16 (0, 4095);
            const cMacAddress& bridgeMac     = getParameterOrOwnMac (params, PAR_STP_BMAC.syntax);
            unsigned portPrio                = params.findParameter (PAR_STP_PPRIO.syntax, (uint32_t)8)->asInt8 (0, 15);
            unsigned portNumber              = params.findParameter (PAR_STP_PNUM.syntax, (uint32_t)1)->asInt16 (1, 4095);
            double msgAge                    = params.findParameter (PAR_STP_MSGAGE.syntax, 0.0)->asDouble (0.0, 255.996);
            double maxAge                    = params.findParameter (PAR_STP_MAXAGE.syntax, 20.0)->asDouble (0.0, 255.996);
            double helloTime                 = params.findParameter (PAR_STP_HELLO.syntax, 2.0)->asDouble (0.0, 255.996);
            double forwardDelay              = params.findParameter (PAR_STP_DELAY.syntax, 15.0)->asDouble (0.0, 255.996);

            flags |= params.findParameter (PAR_STP_TOPOCHANGE.syntax,    (uint32_t)0)->asInt8 (0, 1) ? cStpPacket::TOPO_CHANGE : 0;
            flags |= params.findParameter (PAR_STP_TOPOCHANGEACK.syntax, (uint32_t)0)->asInt8 (0, 1) ? cStpPacket::TOPO_CHANGE_ACK : 0;

            if (isRSTP)
            {
                pathCost      = params.findParameter (PAR_STP_RPATHCOST.syntax, (uint32_t)20000)->asInt32 (1, 4294967295);
                unsigned role = params.findParameter (PAR_STP_PORTROLE.syntax,  (uint32_t)3)->asInt8 (1, 3);

                flags |= params.findParameter (PAR_STP_PROPOSAL.syntax,   (uint32_t)0)->asInt8 (0, 1) ? cStpPacket::PROPOSAL   : 0;
                flags |= params.findParameter (PAR_STP_LEARNING.syntax,   (uint32_t)1)->asInt8 (0, 1) ? cStpPacket::LEARNING   : 0;
                flags |= params.findParameter (PAR_STP_FORWARDING.syntax, (uint32_t)1)->asInt8 (0, 1) ? cStpPacket::FORWARDING : 0;
                flags |= params.findParameter (PAR_STP_AGREEMENT.syntax,  (uint32_t)0)->asInt8 (0, 1) ? cStpPacket::AGREEMENT  : 0;

                stp->compileConfigPduRstp (rootBridgePrio, rootBridgeId, rootBridgeMac, pathCost, bridgePrio, bridgeId,
                        bridgeMac, portPrio, portNumber, msgAge, maxAge, helloTime, forwardDelay, flags, role);
            }
            else
            {
                pathCost = params.findParameter (PAR_STP_RPATHCOST.syntax, (uint32_t)4)->asInt32 (1, 65535);

                stp->compileConfigPdu (rootBridgePrio, rootBridgeId, rootBridgeMac, pathCost, bridgePrio, bridgeId,
                        bridgeMac, portPrio, portNumber, msgAge, maxAge, helloTime, forwardDelay, flags);
            }
        }
        else
        {
            stp->compileTcnPdu ();
        }
    }
    catch (...)
    {
        delete stp;
        stp = nullptr;
        throw;
    }

    return stp;
}


cLinkable* cInstructionParser::compileIGMP  (bool noEthHeader, cParameterList& params, bool v3, bool query, bool report, bool leave)
{
    cIgmpPacket* igmp = new cIgmpPacket;
    try
    {
        cEthernetPacket& eth = igmp->getFirstEthernetPacket ();

        bool destIsMulticast = parseIPv4Params (params, igmp, query || report || leave);
        if (!noEthHeader)
        {
            compileMacHeader  (params, &eth, destIsMulticast || m_ipOptionalDestMAC || query || report || leave);
            compileVLANTags   (params, &eth);
        }
        if (query)
        {
            bool s        = false;
            unsigned qrv  = 0;
            double   qqic = 0;
            double   time = 0;

            if (v3)
            {
                cParameter* optionalPar = nullptr;
                s    = !!params.findParameter (PAR_IGMP_S.syntax, (uint32_t)0)->asInt8 (0, 1);
                qrv  =   params.findParameter (PAR_IGMP_QRV.syntax, (uint32_t)2)->asInt8 (0, 7);
                qqic =   params.findParameter (PAR_IGMP_QQIC.syntax, 125.0)->asDouble (0, 31744.0);
                time =   params.findParameter (PAR_IGMP_TIME.syntax, 10.0)->asDouble (0, 3174.4);

                int sources = 0;
                while ((++sources <= 366 ) &&
                       ((optionalPar = params.findParameter(optionalPar, nullptr, PAR_IGMP_RSIP.syntax, true)) != nullptr))
                {
                    igmp->v3addSource (optionalPar->asIPv4());
                }
            }
            else
            {
                time = params.findParameter (PAR_IGMP_TIME.syntax, 10.0)->asDouble (0, 25.5);
            }
            cParameter* optionalPar = params.findParameter (PAR_IGMP_GROUP.syntax, true);
            if (optionalPar)
                igmp->compileGroupQuery(v3, time, s, qrv, qqic, optionalPar->asIPv4 ());
            else
                igmp->compileGeneralQuery (v3, time, s, qrv, qqic);
        }
        else
        {
            cIPv4 group = params.findParameter (PAR_IGMP_GROUP.syntax)->asIPv4 ();
            if (report)
            {
                igmp->compileReport (group);
            }
            else if (leave)
            {
                igmp->v2compileLeaveGroup (group);
            }
            else    // raw v12 packet
            {
                uint8_t type = params.findParameter (PAR_IGMP_TYPE.syntax)->asInt8();
                uint8_t time = params.findParameter (PAR_IGMP_TIME.syntax, (uint32_t)0)->asInt8();
                igmp->v12compile (type, time, group);
            }
        }
    }
    catch (...)
    {
        delete igmp;
        igmp = nullptr;
        throw;
    }

    return igmp;
}


cLinkable* cInstructionParser::compileICMP  (bool noEthHeader, cParameterList& params)
{
    cIcmpPacket* icmppacket = new cIcmpPacket;
    try
    {
        cEthernetPacket& eth = icmppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, icmppacket);

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader  (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags   (params, &eth);
        }
        uint8_t type = params.findParameter (PAR_ICMP4_TYPE.syntax)->asInt8();
        uint8_t code = params.findParameter (PAR_ICMP4_CODE.syntax)->asInt8();

        size_t len = 0;
        const uint8_t* payload = nullptr;
        cParameter* optionalPar = params.findParameter (PAR_ICMP4_PAYLOAD.syntax, true);
        if (optionalPar)
            payload = optionalPar->asStream(len);

        optionalPar = params.findParameter (PAR_ICMP4_CHKSUM.syntax, true);
        if (optionalPar)
            icmppacket->compileRaw(type, code, optionalPar->asInt16(), payload, len);
        else
            icmppacket->compileRaw(type, code, payload, len);
    }
    catch (...)
    {
        delete icmppacket;
        icmppacket = nullptr;
        throw;
    }

    return icmppacket;
}

cLinkable* cInstructionParser::compileICMPWithEmbedded  (bool noEthHeader, cParameterList& params, uint8_t type)
{
    cIcmpPacket* icmppacket = new cIcmpPacket;
    try
    {
        cEthernetPacket& eth = icmppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, icmppacket);

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader  (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags   (params, &eth);
        }
        uint8_t code = params.findParameter (PAR_ICMP4_CODE.syntax, (uint32_t)0)->asInt8();

        size_t len = 0;
        const uint8_t* payload = nullptr;
        cParameter* optionalPar = params.findParameter (PAR_ICMP4_PAYLOAD.syntax, true);
        if (optionalPar)
            payload = compileEmbedded (optionalPar, true, len);

        icmppacket->compileWithEmbeddedInet(type, code, payload, len);
    }
    catch (...)
    {
        delete icmppacket;
        icmppacket = nullptr;
        throw;
    }

    return icmppacket;
}

cLinkable* cInstructionParser::compileICMPRedirect  (bool noEthHeader, cParameterList& params)
{
    cIcmpPacket* icmppacket = new cIcmpPacket;
    try
    {
        cEthernetPacket& eth = icmppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, icmppacket);

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader  (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags   (params, &eth);
        }
        uint8_t code = params.findParameter (PAR_ICMP4_CODE.syntax, (uint32_t)0)->asInt8();
        const cIPv4 gw = params.findParameter (PAR_ICMP4_GW.syntax)->asIPv4();

        size_t len = 0;
        const uint8_t* payload = nullptr;
        cParameter* optionalPar = params.findParameter (PAR_ICMP4_PAYLOAD.syntax, true);
        if (optionalPar)
            payload = compileEmbedded (optionalPar, true, len);

        icmppacket->compileRedirect (code, gw, payload, len);
    }
    catch (...)
    {
        delete icmppacket;
        icmppacket = nullptr;
        throw;
    }

    return icmppacket;
}

cLinkable* cInstructionParser::compileICMPPing (bool noEthHeader, cParameterList& params, bool reply)
{
    cIcmpPacket* icmppacket = new cIcmpPacket;
    try
    {
        cEthernetPacket& eth = icmppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, icmppacket);

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader  (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags   (params, &eth);
        }
        uint16_t id  = params.findParameter (PAR_ICMP4_ID.syntax,  (uint32_t)0)->asInt16 ();
        uint16_t seq = params.findParameter (PAR_ICMP4_SEQ.syntax, (uint32_t)0)->asInt16 ();

        size_t len = 0;
        const uint8_t* payload = nullptr;
        cParameter* optionalPar = params.findParameter (PAR_ICMP4_PAYLOAD.syntax, true);
        if (optionalPar)
            payload = compileEmbedded (optionalPar, true, len);

        icmppacket->compilePing(reply, id, seq, payload, len);
    }
    catch (...)
    {
        delete icmppacket;
        icmppacket = nullptr;
        throw;
    }

    return icmppacket;
}


cLinkable* cInstructionParser::compileGRE (bool noEthHeader, cParameterList& params, bool isIPv6)
{
    cGrePacket* grepacket = new cGrePacket (isIPv6);
    try
    {
        cEthernetPacket& eth = grepacket->getFirstEthernetPacket();
        bool destIsMulticast = isIPv6 ? parseIPv6Params (params, grepacket) : parseIPv4Params (params, grepacket);

        if (!noEthHeader)
        {
            // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
            compileMacHeader (params, &eth, false, m_ipOptionalDestMAC || destIsMulticast);
            compileVLANTags  (params, &eth);
        }
        grepacket->setProtocolType (params.findParameter (PAR_GRE_PROTOCOL.syntax)->asInt16());

        cParameter* optionalPar;
        optionalPar = params.findParameter (PAR_GRE_KEY.syntax, true);
        if (optionalPar)
            grepacket->setKey (optionalPar->asInt32 ());
        optionalPar = params.findParameter (PAR_GRE_SEQ.syntax, true);
        if (optionalPar)
            grepacket->setSequence (optionalPar->asInt32 ());

        bool calcChecksum = false;
        optionalPar = params.findParameter (PAR_GRE_CHKSUM.syntax, true);
        if (optionalPar)
        {
            uint16_t checksum = optionalPar->asInt16();
            calcChecksum = checksum == 0;
            grepacket->setChecksum (checksum);
        }

        size_t len = 0;
        const uint8_t* payload = nullptr;
        optionalPar = params.findParameter (PAR_GRE_PAYLOAD.syntax, true);
        if (optionalPar)
            payload = compileEmbedded (optionalPar, true, len);
        grepacket->compile (payload, len, calcChecksum);

    }
    catch (...)
    {
        delete grepacket;
        grepacket = nullptr;
        throw;
    }

    return grepacket;
}


cLinkable* cInstructionParser::compileLLDP (bool noEthHeader, cParameterList& params)
{
    cLldpPacket*  lldp = new cLldpPacket;

    /* GOALS: because the amount of parameters is very high
       - lldp() creates a minimal packet with good default values
       - it should be as simple as possible to create "good" lldp packets
       - TLVs must be automatically sorted to get a standard conforming packet
       - in order to allow creation of non-conforming packets, it must be also possible to
         - violate the TLV order
         - leave away mandatory TLVs (including End Of LLDPDU TLV)
         - inject End Of LLDPDU TLV between regular TLVs
       TODO
        - lldp-raw: same parameter, no defaults, compile only tvls where user provided parameters, tlv-order==order of parameters
          --> lldp-raw() will create an empty lldp-packet (no tlvs, no End TLV)
        - 802.3 TLVs (Poe type 3/4, additional caps, poe measurement)
        - lldp-med TLVs (all)
        - Profinet TLVs (NME and 802.1AS)
        - 802.1AB-2023 multiframe PDUs
        - Rethink parameter-naming
        - good defaults for TLVs, maybe a special bool parameter that creates TLV with full defaults
    */

    try
    {
        if (!noEthHeader)
        {
            compileMacHeader (params, lldp, false, true);
            compileVLANTags  (params, lldp);
        }
        cLldpParser parser (*lldp, params);

        // Mandatory IEEE802.1AB TLVs
        parser.chassisID ();
        parser.portID ();
        lldp->addTTL (params.findParameter (PAR_LLDP_TTL.syntax, (uint32_t)120)->asInt16 ());

        // Optional IEEE802.1AB TLVs
        parser.portDescription ();
        parser.systemName ();
        parser.systemDescription ();
        parser.systemCapabilities ();
        parser.managementAddress ();

        // IEEE802.1 TLVs
        parser.portVID ();
        parser.portProtocolVID ();
        parser.vlanName ();
        parser.protocolIdentity ();
        parser.vidUsageDigest ();
        parser.managementVID ();
        parser.linkAggregation ();
        parser.congestionNotification ();
        parser.etsConfiguration ();
        parser.etsRecommendation ();
        parser.pfcControlConfig ();
        parser.applicationPriority ();
        parser.evb ();
        parser.cdcb ();
        parser.applicationVLAN ();

        // IEEE802.3 TLVs
        parser.macPhyConfigStatus ();
        parser.powerViaMDI ();
        parser.maxFrameSize ();
        parser.eee ();
        parser.eeeFastWake ();

        // Profinet TLVs
        parser.pnDelay ();
        parser.pnPortStatus ();
        parser.pnAlias ();
        parser.pnMrpPortState ();
        parser.pnChassisMac ();
        parser.pnPTCPStatus ();
        parser.pnMAUTypeExtension ();
        parser.pnMrpIcPortStatus ();

        // RAW TLVs
        parser.allRawTLVs ();

        // OUI TLVs
        parser.allOidTLVs ();

        lldp->compile ();
    }
    catch (...)
    {
        delete lldp;
        lldp = nullptr;
        throw;
    }

    return lldp;
}


void cInstructionParser::throwParseException (const char* msg, const char* val, size_t valLen, const char* details)
{
    if (details)
        throw ParseException (m_currentInstruction, msg, details, val, (int)valLen);
    else
        throw ParseException (m_currentInstruction, msg, val, (int)valLen);
}


void cInstructionParser::printProtocolList (const char* proto)
{
    for (const auto& p : all_protos)
    {
        if (!proto || !std::strcmp (p->syntax, proto))
        {
            Console::Print ("%-20s%s\n", p->syntax, p->descr);

            if (proto)
            {
                for (const auto& m: p->mandatory)
                {
                    Console::Print ("   %-17s%s\n", m->syntax, m->descr);
                }
                Console::Print (" optional\n");
                for (const auto& o: p->optional)
                {
                    Console::Print ("   %-17s%s\n", o->syntax, o->descr);
                }
                Console::Print ("\n");
                return;
            }
        }
    }
}


#ifdef WITH_UNITTESTS

struct testcase_t
{
    const char* tokens;
    uint8_t packet[cEthernetPacket::MAX_DOUBLE_TAGGED_PACKET];
    size_t packetSize;
};

static const testcase_t tests[] = {
    {
        "eth( dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, ethertype=0x8123, payload=1234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x81, 0x23, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        22,
    },
    {
        "eth(dmac=11:22:33:44:55:66, ethertype=0x8123, payload=1234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x81, 0x23, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        22,
    },
    {
        "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, payload=1234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x08, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        22,
    },
    {
        "eth(dmac=11:22:33:44:55:66, payload=1234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x00, 0x08, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        22,
    },
    {
        "raw(stream = 112233445566aabbccddeeff81231234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x81, 0x23, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        22,
    },
    {
        "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=1, ethertype=0x8123, payload=1234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x81, 0x00, 0x00, 0x01, 0x81, 0x23, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        26,
    },
    {
        "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=42, prio=3, ethertype=0x8123, payload=1234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x81, 0x00, 0x60, 0x2a, 0x81, 0x23, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        26,
    },
    {
        "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, dsap = 0x12, ssap = 0x34, control = 0x11, payload = 1122)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x06, 0x12, 0x34, 0x00, 0x11, 0x11, 0x22
        },
        20,
    },
    {
        "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=42, vtype=1, prio=3, dsap = 0x12, ssap = 0x34, control = 0x11, payload = 1122)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x81, 0x00, 0x60, 0x2a, 0x00, 0x06, 0x12, 0x34, 0x00, 0x11, 0x11, 0x22
        },
        24,
    },
    {
        "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, oui = 0x808182, protocol = 0x34, payload = 1234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x10, 0xaa, 0xaa, 0x03, 0x80, 0x81, 0x82, 0x00, 0x34, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        30,
    },
    {
        "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vid=100, vtype=2, vid=42, prio=3, ethertype=0x8123, payload=1234567890abcdef)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x88, 0xa8, 0x00, 0x64, 0x81, 0x00, 0x60, 0x2a, 0x81, 0x23, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef
        },
        30,
    },
    {
        "arp(op=1, smac=10:22:33:44:55:66, sip=192.168.0.166, dmac=01:02:03:04:05:06, dip=1.2.3.4)",
        {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x10, 0x22, 0x33, 0x44, 0x55, 0x66, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x10, 0x22, 0x33, 0x44, 0x55, 0x66, 0xc0, 0xa8, 0x00, 0xa6, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01, 0x02, 0x03, 0x04
        },
        42,
    },
    {
        "arp(op=1, smac=10:22:33:44:55:66, sip=192.168.0.166, dip=1.2.3.4)",
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x10, 0x22, 0x33, 0x44, 0x55, 0x66, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x10, 0x22, 0x33, 0x44, 0x55, 0x66, 0xc0, 0xa8, 0x00, 0xa6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04
        },
        42,
    },
    { /*13*/
        "arp(dip=11.22.33.44)",
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x16, 0x21, 0x2c
        },
        42,
    },
    {
        "arp(op=2, dip=11.22.33.44)",
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x02, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x16, 0x21, 0x2c
        },
        42,
    },
    {
        "arp(vid=12, dip=11.22.33.44)",
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x81, 0x00, 0x00, 0x0c, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x16, 0x21, 0x2c
        },
        46,
    },
    {
        "arp-probe(dip=11.22.33.44)",
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x16,    0x21, 0x2c
        },
        42,
    },
    {
        "arp-announce(dip=11.22.33.44)",
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x0b, 0x16, 0x21, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x16, 0x21, 0x2c
        },
        42,
    },
    {
        "arp-announce()",
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x0a, 0x0a, 0x0a
        },
        42,
    },
    {/*19*/
        "ipv4(dmac = 11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x08, 0x00, 0x45, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x40, 0xfe, 0x61, 0xcf, 0x0a, 0x0a, 0x0a, 0x0a, 0x01, 0x02, 0x03, 0x04, 0x12, 0x34, 0x56, 0x78
        },
        38,
    },
    {
        "ipv4(vid=42, dmac = 11:22:33:44:55:66, dip=1.2.3.4, protocol=254, payload=12345678)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xba, 0xba, 0xba, 0xba, 0xba, 0xba, 0x81, 0x00, 0x00, 0x2a, 0x08, 0x00, 0x45, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x40, 0xfe, 0x61, 0xcf, 0x0a, 0x0a, 0x0a, 0x0a, 0x01, 0x02, 0x03, 0x04, 0x12, 0x34, 0x56, 0x78
        },
        42,
    },
    {
        "ipv4(smac=80:12:34:45:67:89, dmac = 11:22:33:44:55:66, sip=192.168.0.1, dip=172.16.1.2, ttl=200, dscp=16, ecn=1, df=1, protocol=254, payload=12345678)",
        {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x80, 0x12, 0x34, 0x45, 0x67, 0x89, 0x08, 0x00, 0x45, 0x41, 0x00, 0x18, 0x00, 0x00, 0x40, 0x00, 0xc8, 0xfe, 0x43, 0xeb, 0xc0, 0xa8, 0x00, 0x01, 0xac, 0x10, 0x01, 0x02, 0x12, 0x34, 0x56, 0x78
        },
        38,
    },
    {
        "raw(be16=0x1234, be16=0x1234, le16=0x1234)",
        {
            0x12, 0x34, 0x12, 0x34, 0x34, 0x12
        },
        6
    },
    {
        "raw(byte=0x55, be16=0x1234, le16=0x1234, be32=0x11223344, le32=0x11223344, be64=0x0123456789abcdef, le64=0x0123456789abcdef, ip4=1.2.3.4, ip6=1002:3004:5006:7008:900A:B00C:D00E:F001, mac=10:20:30:40:50:60, stream=\"Hello World\")",
        {
            0x55, 0x12, 0x34, 0x34, 0x12, 0x11, 0x22, 0x33, 0x44, 0x44, 0x33, 0x22, 0x11, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0x01, 0x02, 0x03, 0x04, 0x10, 0x02, 0x30, 0x04, 0x50, 0x06, 0x70, 0x08, 0x90, 0x0a, 0xb0, 0x0c, 0xd0, 0x0e, 0xf0, 0x01, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64
        },
        66
    }

    // FIXME UDP Test Cases
    // FIXME VRRP Test Cases
};


void cInstructionParser::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    bool hasTimestamp;
    cMacAddress ownMac("ba:ba:ba:ba:ba:ba");
    cIPv4 ownIPv4;
    ownIPv4.set("10.10.10.10");
    cSettings::get().setMyMAC(ownMac);
    cSettings::get().setMyIPv4(ownIPv4);
    cInstructionParser obj (false);


    {
        const char s[] = "  abc \tde_0f  ghi";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = obj.parseTimestamp (s, hasTimestamp, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (p == &s[2]);
    }
    {
        const char s[] = "1000";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = obj.parseTimestamp (s, hasTimestamp, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (catched);
        BUG_IF_NOT (!p);
    }
    {
        const char s[] = "+1000";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = obj.parseTimestamp (s, hasTimestamp, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (catched);
        BUG_IF_NOT (!p);
    }
    {
        const char s[] = "1000:";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = obj.parseTimestamp (s, hasTimestamp, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (abs);
        BUG_IF_NOT (p == &s[sizeof(s)]-1);
        BUG_IF_NOT (t == 1000);
    }
    {
        const char s[] = "+1000:";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = obj.parseTimestamp (s, hasTimestamp, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (!abs);
        BUG_IF_NOT (p == &s[sizeof(s)]-1);
        BUG_IF_NOT (t == 1000);
    }
    {
        const char s[] = " 1000 : ";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = obj.parseTimestamp (s, hasTimestamp, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (abs);
        BUG_IF_NOT (p == &s[7]);
        BUG_IF_NOT (t == 1000);
    }
    {
        const char s[] = " +1000 : ";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = obj.parseTimestamp (s, hasTimestamp, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (!abs);
        BUG_IF_NOT (p == &s[8]);
        BUG_IF_NOT (t == 1000);
    }
    {
        const char s[] = " 1asd000 : ";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = obj.parseTimestamp (s, hasTimestamp, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (catched);
        BUG_IF_NOT (abs);
        BUG_IF_NOT (!p);
        BUG_IF_NOT (t==1);
    }
    {
        const char s[] = " abcd(";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = obj.parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (len == 4);
        BUG_IF_NOT (!strncmp("abcd", prot, len));
        BUG_IF_NOT (*p=='(');
    }
    {
        const char s[] = " abcd (";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = obj.parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (len == 4);
        BUG_IF_NOT (!strncmp("abcd", prot, len));
        BUG_IF_NOT (*p=='(');
    }
    {
        const char s[] = "abcd(";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = obj.parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (len == 4);
        BUG_IF_NOT (!strncmp("abcd", prot, len));
        BUG_IF_NOT (*p=='(');
    }
    {
        const char s[] = "abcd( ";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = obj.parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (len == 4);
        BUG_IF_NOT (!strncmp("abcd", prot, len));
        BUG_IF_NOT (*p=='(');
    }
    {
        const char s[] = " ab42cd-ef(";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = obj.parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_IF_NOT (!catched);
        BUG_IF_NOT (len == 9);
        BUG_IF_NOT (!strncmp("ab42cd-ef", prot, len));
        BUG_IF_NOT (*p=='(');
    }
    {
        const char* prot = nullptr;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;
        const char* wrongstrings[] = {
                " ab!42cd_ef(",
                " 1abcd(",
                " _abcd(",
                " -abcd(",
                " abcd "
        };

        for (size_t n = 0; n <  sizeof(wrongstrings) / sizeof(wrongstrings[0]); n++)
        {
            len = 0;
            p = nullptr;
            catched = false;
            prot = nullptr;

            try
            {
                p = obj.parseProtocollIdentifier (wrongstrings[n], &prot, &len);
            }
            catch (ParseException& )
            {
                catched = true;
            }
            BUG_IF_NOT (catched);
            BUG_IF_NOT (!len);
            BUG_IF_NOT (!p);
        }
    }


    //TODO fuzzing of parseTimestamp and parseProtocollIdentifier


    for (unsigned n = 0; n < sizeof(tests)/sizeof(tests[0]); n++)
    {
        cInstructionParser::cResult result;

        Console::PrintDebug("packet %d: ", n);
        cInstructionParser obj2 (false);
        try
        {
            obj2.parse (tests[n].tokens, result);
        }
        catch (ParseException& )
        {
            BUG ("BUG: unexpected exception");
        }
        cEthernetPacket* packets = dynamic_cast<cEthernetPacket*>(result.packets);
        if (!packets)
        {
            cIPPacket* ipv4 = dynamic_cast<cIPPacket*>(result.packets);
            BUG_IF_NOT (ipv4);
            packets = &ipv4->getFirstEthernetPacket();
        }
        BUG_IF_NOT (packets);
        BUG_IF_NOT (tests[n].packetSize == packets->getLength());
        int notEqual = memcmp (packets->get(), tests[n].packet, tests[n].packetSize);
        if (notEqual)
        {
            const uint8_t* p = packets->get();

            for (size_t i = 0; i < packets->getLength(); i++)
            {
                Console::PrintDebug ("0x%02x, ", (int)*p++);
            }
            Console::PrintDebug ("\n");
        }
        BUG_ON (notEqual);
        Console::PrintDebug("\r");

        delete result.packets;
    }
}
#endif /*WITH_UNITTESTS*/
