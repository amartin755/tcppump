/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
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


#include <cstdio>
#include <cassert>
#include <cctype>
#include <cstring>

#include "instructionparser.hpp"

#include "parameterlist.hpp"
#include "protocoltypes.hpp"
#include "ethernetpacket.hpp"
#include "arppacket.hpp"
#include "ipv4packet.hpp"


cInstructionParser::cInstructionParser (mac_t ownMac, ipv4_t ownIPv4)
{
    this->ownMac  = ownMac;
    this->ownIPv4 = ownIPv4;
}


cInstructionParser::~cInstructionParser ()
{
}


int cInstructionParser::parse (const char* instruction, cTimeval& timestamp, bool& isAbsolute, cEthernetPacket& packet)
{
    const char* p = instruction;

    // ignore whitespaces at begin of instruction
    while (isspace (*p))
        p++;

#ifdef WITH_TIMESTAMP
    // if first character is a number we assume there is a timestamp
    if (isdigit (*p) || *p == '+')
    {
        char* end;

        // relative timestamp?
        if (*p == '+')
        {
            isAbsolute = false;
        }
        else
        {
            isAbsolute = true;
        }

        timestamp.setUs ((uint64_t)strtoull (p, &end, 10));
        // check if timestamp is followed by whitespace
        if (!isspace (*end) && *end != '\0')
        {
            throw ParseException ("Invalid timestamp", p);
        }
        p = end;
    }
#else
    timestamp.clear();
    isAbsolute = true;
#endif
    const char* keyword = NULL;
    const char* keywordEnd = NULL;

    // find beginning of protocol keyword
    while (*p != '\0')
    {
        if (isalnum (*p))
        {
            keyword = p;
            break;
        }
        p++;
    }
    if (keyword)
    {
        // find end of protocol keyword
        while (*p++ != '\0')
        {
            if (*p == ':')
            {
                keywordEnd = p++;
                break;
            }
        }
    }

    if (!keyword && !keywordEnd)
        throw ParseException ("Missing protocol specifier", p);
    if (!keywordEnd)
        throw ParseException ("Missing ':' after protocol specifier", keyword);

    // parse protocol parameter list
    cParameterList params (p);
    if (!params.isValid ())
    {
        throw ParseException ("Syntax error", params.getParseError ());
    }

    // compile frames
    try
    {
        //TODO find better way for protocol selection (e.g. hash table)
        if (!strncmp ("raw", keyword, keywordEnd - keyword))
            return compileRAW (params, packet);
        if (!strncmp ("eth", keyword, keywordEnd - keyword))
            return compileETH (params, packet);
        if (!strncmp ("arp", keyword, keywordEnd - keyword))
            return compileARP (params, packet);
        if (!strncmp ("arp-probe", keyword, keywordEnd - keyword))
            return compileARP (params, packet, true);
        if (!strncmp ("arp-announce", keyword, keywordEnd - keyword))
            return compileARP (params, packet, false, true);
        if (!strncmp ("ipv4", keyword, keywordEnd - keyword))
            return compileIPv4 (params, packet);

        throw ParseException ("Unknown protocol type", keyword);
    }
    catch (FormatException& e)
    {
        switch (e.what ())
        {
        case exParUnknown:
            throw ParseException ("Missing parameter", e.value ());
        case exParRange:
            throw ParseException ("Range of parameter violated", e.value ());
        case exParFormat:
            throw ParseException ("Invalid parameter value", e.value ());
        default:
            assert ("BUG: unexpected compile exception" == 0);
        }
    }

    assert ("BUG: unreachable code" == 0);

    return 0;
}


int cInstructionParser::compileRAW (cParameterList& params, cEthernetPacket& eth)
{
    size_t len;
    const char* value = params.findParameter ("payload")->asRaw(len);
    eth.setRaw (value, len);

    return eth.getLength();
}


int cInstructionParser::compileETH (cParameterList& params, cEthernetPacket& eth)
{
    const cParameter* optionalPar = nullptr;

    // MAC header
    // default value of source mac is our own mac address
    eth.setMacHeader (params.findParameter ("src", ownMac)->asMac (),
                      params.findParameter ("dest")->asMac ());


    // compile VLAN tags
    compileVLAN (params, eth);

    // LLC header
    // NOTE: dsap and ssap are mandatory parameters for llc header;
    //       if only one of them is defined, we ignore all LLC parameters
    optionalPar = params.findParameter ("dsap",  true);
    if (optionalPar)
    {
        uint8_t dsap = optionalPar->asInt8 ();
        uint8_t ssap = params.findParameter ("ssap")->asInt8 ();
        eth.addLlcHeader(dsap, ssap, params.findParameter("control", (uint32_t)3)->asInt16 ());
    }
    else
    {
        // SNAP extension
        optionalPar = params.findParameter ("oui",  true);
        if (optionalPar)
        {
            eth.addSnapHeader (optionalPar->asInt32 (0, 0x00ffffff),
                    params.findParameter ("protocol")->asInt16 ());
        }
    }

    size_t len;
    const char* value = params.findParameter ("payload")->asRaw(len);
    eth.setPayload (value, len);

    // if llc header or no ethertype/length is provided, we calculate the length ourself
    if (eth.hasLlcHeader() || (optionalPar = params.findParameter ("ethertype", true)) == NULL)
    {
        eth.setLength ();
    }
    else
    {
        eth.setTypeLength (optionalPar->asInt16 ());
    }


    return eth.getLength();
}


int cInstructionParser::compileVLAN (cParameterList& params, cEthernetPacket& packet)
{
    const cParameter* optionalPar = nullptr;

    // VLAN tags
    while ((optionalPar = params.findParameter(optionalPar, nullptr, "vid", true)) != nullptr)
    {
        packet.addVlanTag ((int)params.findParameter (optionalPar, "vid", "vtype", (uint32_t)1)->asInt8 (1, 2) == 1 ? true : false,
                           (int)optionalPar->asInt16 (0, 0x0fff), // VID
                           (int)params.findParameter (optionalPar, "vid", "prio",  (uint32_t)0)->asInt8 (0, 7),
                           (int)params.findParameter (optionalPar, "vid", "dei",   (uint32_t)0)->asInt8 (0, 1));
    }
    return packet.getLength();
}


int cInstructionParser::compileARP (cParameterList& params, cEthernetPacket& eth, bool isProbe, bool isGratuitous)
{
    cArpPacket arp(eth);
    mac_t targetMac;
    targetMac.set (0);

    assert ((!isProbe && !isGratuitous) || (isProbe != isGratuitous));

    if (isProbe)
    {
        arp.probe (ownMac, params.findParameter ("target_ip")->asIPv4());
    }
    else if (isGratuitous)
    {
        arp.announce (ownMac, params.findParameter ("target_ip", ownIPv4)->asIPv4());
    }
    else
    {
        arp.set (params.findParameter ("op", (uint32_t)1)->asInt16(),
                 params.findParameter ("sender_mac", ownMac)->asMac(),
                 params.findParameter ("sender_ip", ownIPv4)->asIPv4(),
                 params.findParameter ("target_mac", targetMac)->asMac(),
                 params.findParameter ("target_ip")->asIPv4()
                 );
    }

    // compile VLAN tags
    return compileVLAN (params, eth);
}


int cInstructionParser::compileIPv4 (cParameterList& params, cEthernetPacket& eth)
{
    cIPv4Packet packet (eth);

    eth.setMacHeader (params.findParameter ("source_mac", ownMac)->asMac (),
                      params.findParameter ("dest_mac")->asMac ());

    packet.setDSCP         (params.findParameter ("dscp", (uint32_t)0)->asInt8(0, 0x1f));
    packet.setECN          (params.findParameter ("ecn", (uint32_t)0)->asInt8(0, 1));
    packet.setTimeToLive   (params.findParameter ("ttl", (uint32_t)64)->asInt8());
    packet.setDontFragment (params.findParameter ("df", (uint32_t)0)->asInt8(0, 1));
    packet.setDestination  (params.findParameter ("dest")->asIPv4());
    packet.setSource       (params.findParameter ("source", ownIPv4)->asIPv4());

    size_t len;
    const char* payload = params.findParameter ("payload")->asRaw(len);
    packet.setPayload (params.findParameter ("protocol")->asInt8(), payload, len);


    // compile VLAN tags
    return compileVLAN (params, eth);
}

