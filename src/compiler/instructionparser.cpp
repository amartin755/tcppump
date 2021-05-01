// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2021 Andreas Martin (netnag@mailbox.org)
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
#include "ipv4packet.hpp"
#include "udppacket.hpp"
#include "tcppacket.hpp"
#include "vrrppacket.hpp"
#include "stppacket.hpp"
#include "igmppacket.hpp"
#include "icmppacket.hpp"
#include "listener.hpp"
#include "settings.hpp"


cInstructionParser::cInstructionParser (bool optDestMAC)
{
    currentInstruction = nullptr;
    ipOptionalDestMAC = optDestMAC;
}


cInstructionParser::~cInstructionParser ()
{
}

void cInstructionParser::parse (const char* instruction, cResult& result)
{
    currentInstruction = instruction;

    const char* p = instruction;
    const char* keyword;
    size_t      keywordLen;


    p = parseTimestamp (p, result.hasTimestamp, result.timestamp, result.isAbsolute);
    p = parseProtocollIdentifier (p, &keyword, &keywordLen);

    // parse protocol parameter list
    cParameterList params (p);
    if (!params.isValid ())
    {
        throwParseException ("Syntax error", params.getParseError ());
    }

    // compile frames
    try
    {
        //TODO find better way for protocol selection (e.g. hash table)
        if (!strncmp ("raw", keyword, keywordLen))
            result.packets = compileRAW (params);
        else if (!strncmp ("eth", keyword, keywordLen))
            result.packets = compileETH (params);
        else if (!strncmp ("arp", keyword, keywordLen))
            result.packets = compileARP (params);
        else if (!strncmp ("arp-probe", keyword, keywordLen))
            result.packets = compileARP (params, true);
        else if (!strncmp ("arp-announce", keyword, keywordLen))
            result.packets = compileARP (params, false, true);
        else if (!strncmp ("ipv4", keyword, keywordLen))
            result.packets = compileIPv4 (params);
        else if (!strncmp ("udp", keyword, keywordLen))
            result.packets = compileUDP (params);
        else if (!strncmp ("vrrp", keyword, keywordLen))
            result.packets = compileVRRP (params, 2);
        else if (!strncmp ("vrrp3", keyword, keywordLen))
            result.packets = compileVRRP (params, 3);
        else if (!strncmp ("stp", keyword, keywordLen))
            result.packets = compileSTP (params);
        else if (!strncmp ("stp-tcn", keyword, keywordLen))
            result.packets = compileSTP (params, false, true);
        else if (!strncmp ("rstp", keyword, keywordLen))
            result.packets = compileSTP (params, true);
        else if (!strncmp ("igmp", keyword, keywordLen))
            result.packets = compileIGMP (params, false, false, false, false);
        else if (!strncmp ("igmp-query", keyword, keywordLen))
            result.packets = compileIGMP (params, false, true, false, false);
        else if (!strncmp ("igmp3-query", keyword, keywordLen))
            result.packets = compileIGMP (params, true, true, false, false);
        else if (!strncmp ("igmp-report", keyword, keywordLen))
            result.packets = compileIGMP (params, false, false, true, false);
        else if (!strncmp ("igmp-leave", keyword, keywordLen))
            result.packets = compileIGMP (params, false, false, false, true);
        else if (!strncmp ("icmp", keyword, keywordLen))
            result.packets = compileICMP (params);
        else if (!strncmp ("tcp", keyword, keywordLen))
            result.packets = compileTCP (params);
        else if (!strncmp ("tcp-syn", keyword, keywordLen))
            result.packets = compileTCPSYN (params);
        else if (!strncmp ("tcp-syn-ack", keyword, keywordLen))
            result.packets = compileTCPSYNACK (params);
        else if (!strncmp ("tcp-syn-ack2", keyword, keywordLen))
            result.packets = compileTCPSYNACK2 (params);
        else if (!strncmp ("tcp-fin", keyword, keywordLen))
            result.packets = compileTCPFIN (params);
        else if (!strncmp ("tcp-fin-ack", keyword, keywordLen))
            result.packets = compileTCPFINACK (params);
        else if (!strncmp ("tcp-fin-ack2", keyword, keywordLen))
            result.packets = compileTCPFINACK2 (params);
        else if (!strncmp ("tcp-reset", keyword, keywordLen))
            result.packets = compileTCPRST (params);
        else if (!strncmp ("LISTEN", keyword, keywordLen))
            result.packets = compileListen (params);
        else
            throwParseException ("Unknown protocol type", keyword, keywordLen);

        params.checkForUnusedParameters ();

        return;
    }
    catch (FormatException& e)
    {
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


cLinkable* cInstructionParser::compileRAW (cParameterList& params)
{
    size_t len;
    const uint8_t* value = params.findParameter ("payload")->asStream(len);
    cEthernetPacket* eth = new cEthernetPacket (len);
    eth->setRaw (value, len);

    return eth; // one packet was added to the list
}


// returns true, if destination MAC address is set
bool cInstructionParser::compileMacHeader (cParameterList& params, cEthernetPacket *packet, bool noDestination, bool destIsOptional)
{
    // default value of source mac is our own mac address
    packet->setSrcMac (getParameterOrOwnMac (params, "smac"));

    if (!noDestination)
    {
        const cParameter* destMacPar = params.findParameter ("dmac", destIsOptional);
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


cIpAddress cInstructionParser::getParameterOrOwnIPv4 (cParameterList& params, const char* par) const
{
    const cParameter* optionalPar = params.findParameter (par, true);

    return optionalPar ? optionalPar->asIPv4() : cSettings::get().getMyIPv4();
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
        optionalPar = params.findParameter ("dsap",  true);
        if (optionalPar)
        {
            uint8_t dsap = optionalPar->asInt8 ();
            uint8_t ssap = params.findParameter ("ssap")->asInt8 ();
            eth->addLlcHeader(dsap, ssap, params.findParameter("control", (uint32_t)3)->asInt16 ());
        }
        else
        {
            // SNAP extension
            optionalPar = params.findParameter ("oui",  true);
            if (optionalPar)
            {
                eth->addSnapHeader (optionalPar->asInt32 (0, 0x00ffffff),
                        params.findParameter ("protocol")->asInt16 ());
            }
        }

        size_t len;
        const uint8_t* value = params.findParameter ("payload")->asStream(len);
        eth->setPayload (value, len);

        // if llc header or no ethertype/length is provided, we calculate the length ourself
        if (eth->hasLlcHeader() || (optionalPar = params.findParameter ("ethertype", true)) == NULL)
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
    while ((optionalPar = params.findParameter(optionalPar, nullptr, "vid", true)) != nullptr)
    {
        packet->addVlanTag ((int)params.findParameter (optionalPar, "vid", "vtype", (uint32_t)1)->asInt8 (1, 2) == 1 ? true : false,
                           (int)optionalPar->asInt16 (0, 0x0fff), // VID
                           (int)params.findParameter (optionalPar, "vid", "prio",  (uint32_t)0)->asInt8 (0, 7),
                           (int)params.findParameter (optionalPar, "vid", "dei",   (uint32_t)0)->asInt8 (0, 1));
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
            arp->probe (cSettings::get().getMyMAC(), params.findParameter ("dip")->asIPv4());
        }
        else if (isGratuitous)
        {
            arp->announce (cSettings::get().getMyMAC(), getParameterOrOwnIPv4 (params, "dip"));
        }
        else
        {
            cMacAddress targetMac = params.findParameter ("dmac", cMacAddress ((unsigned)0))->asMac();

            arp->setAll (params.findParameter ("op", (uint32_t)1)->asInt16(),
                         getParameterOrOwnMac (params, "smac"),
                         getParameterOrOwnIPv4 (params, "sip"),
                         targetMac,
                         params.findParameter ("dip")->asIPv4()
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
bool cInstructionParser::parseIPv4Params (cParameterList& params, cIPv4Packet* packet, bool noDestinationIP)
{
    bool isMulticast = false;

    packet->setDSCP         (params.findParameter ("dscp", (uint32_t)0)->asInt8(0, 0x3f));
    packet->setECN          (params.findParameter ("ecn", (uint32_t)0)->asInt8(0, 3));
    packet->setTimeToLive   (params.findParameter ("ttl", (uint32_t)64)->asInt8());
    packet->setDontFragment (params.findParameter ("df", (uint32_t)0)->asInt8(0, 1));
    if (!noDestinationIP)
    {
        const cIpAddress destIP = params.findParameter ("dip")->asIPv4();
        packet->setDestination (destIP);
        isMulticast = destIP.isMulticast();
    }
    packet->setSource (getParameterOrOwnIPv4 (params, "sip"));
    cParameter* optionalPar = params.findParameter ("id", true);
    if (optionalPar)
        packet->setIdentification(optionalPar->asInt16());

    return isMulticast;
}


cLinkable* cInstructionParser::compileIPv4 (cParameterList& params)
{
    cIPv4Packet* ippacket = new cIPv4Packet;
    try
    {
        cEthernetPacket& eth = ippacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, ippacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        size_t len;
        const uint8_t* payload = params.findParameter ("payload")->asStream(len);
        ippacket->compile (params.findParameter ("protocol")->asInt8(), nullptr, 0, payload, len);
    }
    catch (...)
    {
        delete ippacket;
        ippacket = nullptr;
        throw;
    }
    return ippacket;
}


cLinkable* cInstructionParser::compileUDP (cParameterList& params)
{
    cUdpPacket* udppacket = new cUdpPacket;
    try
    {
        cEthernetPacket& eth = udppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, udppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        udppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        udppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        size_t len = 0;
        const uint8_t* payload = nullptr;
        cParameter* optionalPar = params.findParameter ("payload", true);
        if (optionalPar)
            payload = optionalPar->asStream(len);
        udppacket->setPayload (payload, len);

        optionalPar = params.findParameter ("chksum", true);
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


cLinkable* cInstructionParser::compileTCP (cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);
        bool userDefinedChecksum = false;
        cParameter* optionalPar;

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        tcppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        tcppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        tcppacket->setSeqNumber (params.findParameter ("seq")->asInt32());
        tcppacket->setAckNumber (params.findParameter ("ack")->asInt32());

        tcppacket->setWindow (params.findParameter ("win", (uint32_t)1024)->asInt16());
        tcppacket->setUrgentPointer (params.findParameter ("urgptr", (uint32_t)0)->asInt16());

        // Flags
        tcppacket->setFlagFIN (!!(params.findParameter ("FIN",    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagSYN (!!(params.findParameter ("SYN",    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagRST (!!(params.findParameter ("RESET",  (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagPSH (!!(params.findParameter ("PUSH",   (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagACK (!!(params.findParameter ("ACK",    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagURG (!!(params.findParameter ("URGENT", (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagECE (!!(params.findParameter ("ECN",    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagCWR (!!(params.findParameter ("CWR",    (uint32_t)0)->asInt8(0, 1)));
        tcppacket->setFlagNON (!!(params.findParameter ("NONCE",  (uint32_t)0)->asInt8(0, 1)));

        size_t len = 0;
        const uint8_t* payload = nullptr;
        optionalPar = params.findParameter ("payload", true);
        if (optionalPar)
            payload = optionalPar->asStream(len);

        optionalPar = params.findParameter ("chksum", true);
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


cLinkable* cInstructionParser::compileTCPSYN (cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        tcppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        tcppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        tcppacket->setSeqNumber (0);
        tcppacket->setAckNumber (0);
        tcppacket->setWindow (1024);
        tcppacket->setFlagSYN (true);

        tcppacket->compile (nullptr, 0, true);
    }
    catch (...)
    {
        delete tcppacket;
        tcppacket = nullptr;
        throw;
    }

    return tcppacket;
}


cLinkable* cInstructionParser::compileTCPSYNACK (cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        tcppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        tcppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        tcppacket->setSeqNumber (0);
        tcppacket->setAckNumber (1);
        tcppacket->setWindow (1024);
        tcppacket->setFlagSYN (true);
        tcppacket->setFlagACK (true);

        tcppacket->compile (nullptr, 0, true);
    }
    catch (...)
    {
        delete tcppacket;
        tcppacket = nullptr;
        throw;
    }

    return tcppacket;
}


cLinkable* cInstructionParser::compileTCPSYNACK2 (cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        tcppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        tcppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        tcppacket->setSeqNumber (1);
        tcppacket->setAckNumber (1);
        tcppacket->setWindow (1024);
        tcppacket->setFlagACK (true);

        tcppacket->compile (nullptr, 0, true);
    }
    catch (...)
    {
        delete tcppacket;
        tcppacket = nullptr;
        throw;
    }

    return tcppacket;
}


cLinkable* cInstructionParser::compileTCPFIN (cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        tcppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        tcppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        tcppacket->setSeqNumber (1);
        tcppacket->setAckNumber (1);
        tcppacket->setWindow (1024);
        tcppacket->setFlagFIN (true);
        tcppacket->setFlagACK (true);

        tcppacket->compile (nullptr, 0, true);
    }
    catch (...)
    {
        delete tcppacket;
        tcppacket = nullptr;
        throw;
    }

    return tcppacket;
}


cLinkable* cInstructionParser::compileTCPFINACK (cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        tcppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        tcppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        tcppacket->setSeqNumber (1);
        tcppacket->setAckNumber (2);
        tcppacket->setWindow (1024);
        tcppacket->setFlagFIN (true);
        tcppacket->setFlagACK (true);

        tcppacket->compile (nullptr, 0, true);
    }
    catch (...)
    {
        delete tcppacket;
        tcppacket = nullptr;
        throw;
    }

    return tcppacket;
}


cLinkable* cInstructionParser::compileTCPFINACK2 (cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        tcppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        tcppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        tcppacket->setSeqNumber (2);
        tcppacket->setAckNumber (2);
        tcppacket->setWindow (1024);
        tcppacket->setFlagACK (true);

        tcppacket->compile (nullptr, 0, true);
    }
    catch (...)
    {
        delete tcppacket;
        tcppacket = nullptr;
        throw;
    }

    return tcppacket;
}


cLinkable* cInstructionParser::compileTCPRST (cParameterList& params)
{
    cTcpPacket* tcppacket = new cTcpPacket;
    try
    {
        cEthernetPacket& eth = tcppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, tcppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        tcppacket->setSourcePort(params.findParameter ("sport")->asInt16());
        tcppacket->setDestinationPort(params.findParameter ("dport")->asInt16());

        tcppacket->setSeqNumber (0);
        tcppacket->setAckNumber (0);
        tcppacket->setWindow (1024);
        tcppacket->setFlagRST (true);

        tcppacket->compile (nullptr, 0, true);
    }
    catch (...)
    {
        delete tcppacket;
        tcppacket = nullptr;
        throw;
    }

    return tcppacket;
}


cLinkable* cInstructionParser::compileVRRP (cParameterList& params, int version)
{
    cVrrpPacket* vrrp = new cVrrpPacket;
    try
    {
        bool userDefinedChecksum = false;
        cEthernetPacket& eth = vrrp->getFirstEthernetPacket();

        parseIPv4Params   (params, vrrp, true);
        compileMacHeader  (params, &eth, true);
        compileVLANTags   (params, &eth);

        const cParameter* firstVRIP = params.findParameter ("vrip");

        vrrp->setVersion(version);
        vrrp->setVRID(params.findParameter ("vrid")->asInt8(1, 255));
        vrrp->addVirtualIP(firstVRIP->asIPv4());
        vrrp->setPrio(params.findParameter ("vrprio", (uint32_t)100)->asInt8());
        vrrp->setType(params.findParameter ("type", (uint32_t)1)->asInt8(0, 15));
        if (version == 2)
            vrrp->setInterval(params.findParameter ("aint", (uint32_t)1)->asInt8());
        else
            vrrp->setInterval(params.findParameter ("aint", (uint32_t)100)->asInt16(0, 4095));
        const cParameter* optionalPar = params.findParameter ("chksum", true);
        if (optionalPar)
        {
            userDefinedChecksum = true;
            vrrp->setChecksum (optionalPar->asInt16());
        }

        // additional virtual IPs (optional)
        optionalPar = firstVRIP;
        int vripCount = 1;
        while ((++vripCount <= 255 ) &&
               ((optionalPar = params.findParameter(optionalPar, nullptr, "vrip", true)) != nullptr))
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


cLinkable* cInstructionParser::compileSTP (cParameterList& params, bool isRSTP, bool isTCN)
{
    cStpPacket*  stp = new cStpPacket;

    try
    {
        compileMacHeader (params, stp, true);
        compileVLANTags  (params, stp);

        if (!isTCN)
        {
            int flags = 0;
            uint32_t pathCost;
            unsigned rootBridgePrio          = params.findParameter ("rbprio", (uint32_t)8)->asInt8 (0, 15);
            unsigned rootBridgeId            = params.findParameter ("rbidext", (uint32_t)0)->asInt16 (0, 4095);
            const cMacAddress& rootBridgeMac = getParameterOrOwnMac (params, "rbmac");
            unsigned bridgePrio              = params.findParameter ("bprio", (uint32_t)8)->asInt8 (0, 15);
            unsigned bridgeId                = params.findParameter ("bidext", (uint32_t)0)->asInt16 (0, 4095);
            const cMacAddress& bridgeMac     = getParameterOrOwnMac (params, "bmac");
            unsigned portPrio                = params.findParameter ("pprio", (uint32_t)8)->asInt8 (0, 15);
            unsigned portNumber              = params.findParameter ("pnum", (uint32_t)1)->asInt16 (1, 4095);
            double msgAge                    = params.findParameter ("msgage", 0.0)->asDouble (0.0, 255.996);
            double maxAge                    = params.findParameter ("maxage", 20.0)->asDouble (0.0, 255.996);
            double helloTime                 = params.findParameter ("hello", 2.0)->asDouble (0.0, 255.996);
            double forwardDelay              = params.findParameter ("delay", 15.0)->asDouble (0.0, 255.996);

            flags |= params.findParameter ("topochange",    (uint32_t)0)->asInt8 (0, 1) ? cStpPacket::TOPO_CHANGE : 0;
            flags |= params.findParameter ("topochangeack", (uint32_t)0)->asInt8 (0, 1) ? cStpPacket::TOPO_CHANGE_ACK : 0;

            if (isRSTP)
            {
                pathCost      = params.findParameter ("rpathcost", (uint32_t)20000)->asInt32 (1, 4294967295);
                unsigned role = params.findParameter ("portrole",  (uint32_t)3)->asInt8 (1, 3);

                flags |= params.findParameter ("proposal",   (uint32_t)0)->asInt8 (0, 1) ? cStpPacket::PROPOSAL   : 0;
                flags |= params.findParameter ("learning",   (uint32_t)1)->asInt8 (0, 1) ? cStpPacket::LEARNING   : 0;
                flags |= params.findParameter ("forwarding", (uint32_t)1)->asInt8 (0, 1) ? cStpPacket::FORWARDING : 0;
                flags |= params.findParameter ("agreement",  (uint32_t)0)->asInt8 (0, 1) ? cStpPacket::AGREEMENT  : 0;

                stp->compileConfigPduRstp (rootBridgePrio, rootBridgeId, rootBridgeMac, pathCost, bridgePrio, bridgeId,
                        bridgeMac, portPrio, portNumber, msgAge, maxAge, helloTime, forwardDelay, flags, role);
            }
            else
            {
                pathCost = params.findParameter ("rpathcost", (uint32_t)4)->asInt32 (1, 65535);

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


cLinkable* cInstructionParser::compileIGMP  (cParameterList& params, bool v3, bool query, bool report, bool leave)
{
    cIgmpPacket* igmp = new cIgmpPacket;
    try
    {
        cEthernetPacket& eth = igmp->getFirstEthernetPacket ();

        bool destIsMulticast = parseIPv4Params (params, igmp, query || report || leave);
        compileMacHeader  (params, &eth, destIsMulticast || ipOptionalDestMAC || query || report || leave);
        compileVLANTags   (params, &eth);

        if (query)
        {
            bool s        = false;
            unsigned qrv  = 0;
            double   qqic = 0;
            double   time = 0;

            if (v3)
            {
                cParameter* optionalPar = nullptr;
                s    = !!params.findParameter ("s", (uint32_t)0)->asInt8 (0, 1);
                qrv  =   params.findParameter ("qrv", (uint32_t)2)->asInt8 (0, 7);
                qqic =   params.findParameter ("qqic", 125.0)->asDouble (0, 31744.0);
                time =   params.findParameter ("time", 10.0)->asDouble (0, 3174.4);

                int sources = 0;
                while ((++sources <= 366 ) &&
                       ((optionalPar = params.findParameter(optionalPar, nullptr, "rsip", true)) != nullptr))
                {
                    igmp->v3addSource (optionalPar->asIPv4());
                }
            }
            else
            {
                time = params.findParameter ("time", 10.0)->asDouble (0, 25.5);
            }
            cParameter* optionalPar = params.findParameter ("group", true);
            if (optionalPar)
                igmp->compileGroupQuery(v3, time, s, qrv, qqic, optionalPar->asIPv4 ());
            else
                igmp->compileGeneralQuery (v3, time, s, qrv, qqic);
        }
        else
        {
            cIpAddress group = params.findParameter ("group")->asIPv4 ();
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
                uint8_t type = params.findParameter ("type")->asInt8();
                uint8_t time = params.findParameter ("time", (uint32_t)0)->asInt8();
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


cLinkable* cInstructionParser::compileICMP  (cParameterList& params)
{
    cIcmpPacket* icmppacket = new cIcmpPacket;
    try
    {
        cEthernetPacket& eth = icmppacket->getFirstEthernetPacket();
        bool destIsMulticast = parseIPv4Params (params, icmppacket);

        // --> dest mac is set automatically, if dest IP is a multicast OR user has NOT provided a dest MAC
        compileMacHeader  (params, &eth, false, ipOptionalDestMAC || destIsMulticast);
        compileVLANTags   (params, &eth);

        uint8_t type = params.findParameter ("type")->asInt8();
        uint8_t code = params.findParameter ("code")->asInt8();

        size_t len = 0;
        const uint8_t* payload = nullptr;
        cParameter* optionalPar = params.findParameter ("payload", true);
        if (optionalPar)
            payload = optionalPar->asStream(len);

        optionalPar = params.findParameter ("chksum", true);
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


cLinkable* cInstructionParser::compileListen (cParameterList& params)
{
    cListener* event = new cListener;
    try
    {
        cParameter* optionalPar;
        // TODO
        // - new parameter "pattern=STREAM" memmem(STREAM)
        // - new parameter "packet="eth(kdkd, dlsl)" which compiles the provided embedded packet
        // - packet and pattern can be combined with bpf, whereas bpf has to match first
        // - only packet OR pattern are possible. they exclude each other
        // - new parameter timeout

        optionalPar = params.findParameter ("bpf", true);
        if (optionalPar)
        {
            size_t len;
            const char* p = (const char*)optionalPar->asStream (len);
            std::string s (p, len);
            if (!event->compileBpfFilter (s.c_str()))
                optionalPar->throwValueExcetion ();
        }
        optionalPar = params.findParameter ("pattern", true);
        if (optionalPar)
        {
            size_t len = 0;
            const uint8_t* pattern = optionalPar->asStream(len);
            event->setPatternFilter (pattern, len);
        }
        event->setTimeout (params.findParameter ("timeout", (uint32_t)0)->asInt32());
    }
    catch (...)
    {
        delete event;
        event = nullptr;
        throw;
    }
    return event;
}


void cInstructionParser::throwParseException (const char* msg, const char* val, size_t valLen, const char* details)
{
    if (details)
        throw ParseException (currentInstruction, msg, details, val, (int)valLen);
    else
        throw ParseException (currentInstruction, msg, val, (int)valLen);
}


#ifdef WITH_UNITTESTS

#include "console.hpp"

typedef struct
{
    const char* tokens;
    uint8_t packet[cEthernetPacket::MAX_DOUBLE_TAGGED_PACKET];
    size_t packetSize;
}testcase_t;

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
        "raw(payload = 112233445566aabbccddeeff81231234567890abcdef)",
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

    // FIXME UDP Test Cases
    // FIXME VRRP Test Cases
};


void cInstructionParser::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    bool hasTimestamp;
    cMacAddress ownMac("ba:ba:ba:ba:ba:ba");
    cIpAddress ownIPv4;
    ownIPv4.set("10.10.10.10");
    cSettings::get().setMyMAC(ownMac);
    cSettings::get().setMyIPv4(ownIPv4);
    std::list <cEthernetPacket> packets;
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
        assert (!catched);
        assert (p == &s[2]);
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
        assert (catched);
        assert (!p);
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
        assert (catched);
        assert (!p);
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
        assert (!catched);
        assert (abs);
        assert (p == &s[sizeof(s)]-1);
        assert (t == 1000);
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
        assert (!catched);
        assert (!abs);
        assert (p == &s[sizeof(s)]-1);
        assert (t == 1000);
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
        assert (!catched);
        assert (abs);
        assert (p == &s[7]);
        assert (t == 1000);
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
        assert (!catched);
        assert (!abs);
        assert (p == &s[8]);
        assert (t == 1000);
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
        assert (catched);
        assert (abs);
        assert (!p);
        assert (t==1);
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
        assert (!catched);
        assert (len == 4);
        assert (!strncmp("abcd", prot, len));
        assert (*p=='(');
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
        assert (!catched);
        assert (len == 4);
        assert (!strncmp("abcd", prot, len));
        assert (*p=='(');
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
        assert (!catched);
        assert (len == 4);
        assert (!strncmp("abcd", prot, len));
        assert (*p=='(');
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
        assert (!catched);
        assert (len == 4);
        assert (!strncmp("abcd", prot, len));
        assert (*p=='(');
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
        assert (!catched);
        assert (len == 9);
        assert (!strncmp("ab42cd-ef", prot, len));
        assert (*p=='(');
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
            assert (catched);
            assert (!len);
            assert (!p);
        }
    }


    //TODO fuzzing of parseTimestamp and parseProtocollIdentifier


    for (unsigned n = 0; n < sizeof(tests)/sizeof(tests[0]); n++)
    {
        cInstructionParser::cResult result;

        Console::PrintDebug("packet %d", n);
        cInstructionParser obj (false);
        try
        {
            obj.parse (tests[n].tokens, result);
        }
        catch (ParseException& )
        {
            BUG ("BUG: unexpected exception");
        }
        cEthernetPacket* packets = dynamic_cast<cEthernetPacket*>(result.packets);
        if (!packets)
        {
            cIPv4Packet* ipv4 = dynamic_cast<cIPv4Packet*>(result.packets);
            assert (ipv4);
            packets = &ipv4->getFirstEthernetPacket();
        }
        assert (packets);
        assert (tests[n].packetSize == packets->getLength());
        if (memcmp (packets->get(), tests[n].packet, tests[n].packetSize))
        {
            const uint8_t* p = packets->get();

            for (size_t n = 0; n < packets->getLength(); n++)
            {
                printf ("0x%02x, ", (int)*p++);
            }
            printf ("\n");
        }
        assert (!memcmp (packets->get(), tests[n].packet, tests[n].packetSize));
        Console::PrintDebug("\r");

        delete result.packets;
    }
}
#endif /*WITH_UNITTESTS*/
