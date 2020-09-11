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


#include <cstdio>
#include <cctype>
#include <cstring>

#include "bugon.h"
#include "instructionparser.hpp"

#include "parameterlist.hpp"
#include "ethernetpacket.hpp"
#include "arppacket.hpp"
#include "ipv4packet.hpp"
#include "udppacket.hpp"
#include "vrrppacket.hpp"
#include "parsehelper.hpp"


cInstructionParser::cInstructionParser (const cMacAddress& ownMac, const cIpAddress& ownIPv4)
{
    this->ownMac.set(ownMac);
    this->ownIPv4.set(ownIPv4);
}


cInstructionParser::~cInstructionParser ()
{
}

// returns the number of added packets to the list 'packets'
int cInstructionParser::parse (const char* instruction, uint64_t& timestamp, bool& isAbsolute,  std::list <cEthernetPacket> &packets)
{
    const char* p = instruction;
    const char* keyword;
    size_t      keywordLen;

    p = parseTimestamp (p, timestamp, isAbsolute);
    p = parseProtocollIdentifier (p, &keyword, &keywordLen);

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
        if (!strncmp ("raw", keyword, keywordLen))
            return compileRAW (params, packets);
        if (!strncmp ("eth", keyword, keywordLen))
            return compileETH (params, packets);
        if (!strncmp ("arp", keyword, keywordLen))
            return compileARP (params, packets);
        if (!strncmp ("arp-probe", keyword, keywordLen))
            return compileARP (params, packets, true);
        if (!strncmp ("arp-announce", keyword, keywordLen))
            return compileARP (params, packets, false, true);
        if (!strncmp ("ipv4", keyword, keywordLen))
            return compileIPv4 (params, packets);
        if (!strncmp ("udp", keyword, keywordLen))
            return compileUDP (params, packets);
        if (!strncmp ("vrrp", keyword, keywordLen))
            return compileVRRP (params, packets);

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
            BUG_ON ("BUG: unexpected compile exception" == 0);
        }
    }

    BUG_ON ("BUG: unreachable code" == 0);

    return 0;
}

const char* cInstructionParser::parseTimestamp (const char* p, uint64_t& timestamp, bool& isAbsolute)
{
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
                throw ParseException ("Expected ':' after timestamp", end);
            p++;
        }
        else
        {
            throw ParseException ("Invalid timestamp", p);
        }
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
        throw ParseException ("Missing protocol specifier", p);

    // find begin of parameter list --> '('
    p = cParseHelper::nextCharIgnoreWhitspaces (p, '(');
    if (!p)
        throw ParseException ("Expected '(' after protocol specifier", keyword);



    *identifier = keyword;
    *len = keywordEnd - keyword;

    return p;
}


int cInstructionParser::compileRAW (cParameterList& params, std::list <cEthernetPacket> &packets)
{
    size_t len;
    const uint8_t* value = params.findParameter ("payload")->asStream(len);
    cEthernetPacket eth;
    eth.setRaw (value, len);
    packets.push_back (std::move(eth));

    return 1; // one packet was added to the list
}


void cInstructionParser::compileMacHeader (cParameterList& params, cEthernetPacket &packet)
{
    // default value of source mac is our own mac address
    packet.setMacHeader (params.findParameter ("smac", ownMac)->asMac (),
                         params.findParameter ("dmac")->asMac ());
}


int cInstructionParser::compileETH (cParameterList& params, std::list <cEthernetPacket> &packets)
{
    cEthernetPacket eth;
    const cParameter* optionalPar = nullptr;

    // MAC header
    compileMacHeader (params, eth);

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
    const uint8_t* value = params.findParameter ("payload")->asStream(len);
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


    packets.push_back (std::move(eth));
    return 1; // one packet was added to the list
}


size_t cInstructionParser::compileVLANTags (cParameterList& params, cEthernetPacket &packet)
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


int cInstructionParser::compileARP (cParameterList& params, std::list <cEthernetPacket> &packets, bool isProbe, bool isGratuitous)
{
    cArpPacket  arp;

    BUG_ON ((!isProbe && !isGratuitous) || (isProbe != isGratuitous));

    if (isProbe)
    {
        arp.probe (ownMac, params.findParameter ("dip")->asIPv4());
    }
    else if (isGratuitous)
    {
        arp.announce (ownMac, params.findParameter ("dip", ownIPv4)->asIPv4());
    }
    else
    {
        cMacAddress targetMac = params.findParameter ("dmac", cMacAddress ((unsigned)0))->asMac();

        arp.setAll (params.findParameter ("op", (uint32_t)1)->asInt16(),
                    params.findParameter ("smac", ownMac)->asMac(),
                    params.findParameter ("sip", ownIPv4)->asIPv4(),
                    targetMac,
                    params.findParameter ("dip")->asIPv4()
                    );
    }

    // compile VLAN tags
    compileVLANTags (params, arp);
    packets.push_back (std::move(arp));

    return 1; // one packet was added to the list
}


void cInstructionParser::compileIPv4Header (cParameterList& params, cIPv4Packet& packet, bool noDestinationIP)
{
    packet.setDSCP         (params.findParameter ("dscp", (uint32_t)0)->asInt8(0, 0x3f));
    packet.setECN          (params.findParameter ("ecn", (uint32_t)0)->asInt8(0, 3));
    packet.setTimeToLive   (params.findParameter ("ttl", (uint32_t)64)->asInt8());
    packet.setDontFragment (params.findParameter ("df", (uint32_t)0)->asInt8(0, 1));
    if (!noDestinationIP)
        packet.setDestination  (params.findParameter ("dip")->asIPv4());
    packet.setSource       (params.findParameter ("sip", ownIPv4)->asIPv4());
}


int cInstructionParser::compileIPv4 (cParameterList& params, std::list <cEthernetPacket> &packets)
{
    cIPv4Packet ippacket;
    cEthernetPacket& eth = ippacket.getFirstEthernetPacket();

    compileMacHeader  (params, eth);
    compileVLANTags   (params, eth);
    compileIPv4Header (params, ippacket);

    size_t len;
    const uint8_t* payload = params.findParameter ("payload")->asStream(len);
    ippacket.setPayload (params.findParameter ("protocol")->asInt8(), nullptr, 0, payload, len);

    return (int)ippacket.getAllEthernetPackets(packets);
}


int cInstructionParser::compileUDP (cParameterList& params, std::list <cEthernetPacket> &packets)
{
    cUdpPacket udppacket;
    cEthernetPacket& eth = udppacket.getFirstEthernetPacket();

    compileMacHeader  (params, eth);
    compileVLANTags   (params, eth);
    compileIPv4Header (params, udppacket);

    udppacket.setSourcePort(params.findParameter ("sport")->asInt16());
    udppacket.setDestinationPort(params.findParameter ("dport")->asInt16());

    size_t len = 0;
    const uint8_t* payload = nullptr;
    cParameter* optionalPar = params.findParameter ("payload", true);
    if (optionalPar)
        payload = optionalPar->asStream(len);
    udppacket.setPayload (payload, len);

    optionalPar = params.findParameter ("chksum", true);
    if (optionalPar)
        udppacket.setChecksum (optionalPar->asInt16());

    return (int)udppacket.getAllEthernetPackets(packets);
}


int cInstructionParser::compileVRRP (cParameterList& params, std::list <cEthernetPacket> &packets)
{
    bool userDefinedChecksum = false;
    cVrrpPacket vrrp;
    cEthernetPacket& eth = vrrp.getFirstEthernetPacket();

    compileVLANTags   (params, eth);
    compileIPv4Header (params, vrrp, true);

    uint8_t version = params.findParameter ("vers", (uint32_t)3)->asInt8(2, 3);
    const cParameter* firstVRIP = params.findParameter ("vrip");

    vrrp.setVersion(version);
    vrrp.setVRID(params.findParameter ("vrid")->asInt8(1, 255));
    vrrp.addVirtualIP(firstVRIP->asIPv4());
    vrrp.setPrio(params.findParameter ("prio", (uint32_t)100)->asInt8());
    vrrp.setType(params.findParameter ("type", (uint32_t)1)->asInt8());
    if (version == 2)
        vrrp.setInterval(params.findParameter ("aint", (uint32_t)1)->asInt8());
    else
        vrrp.setInterval(params.findParameter ("aint", (uint32_t)100)->asInt16(0, 4095));
    const cParameter* optionalPar = params.findParameter ("chksum", true);
    if (optionalPar)
    {
        userDefinedChecksum = true;
        vrrp.setChecksum (optionalPar->asInt16());
    }

    // additional virtual IPs (optional)
    optionalPar = firstVRIP;
    int vripCount = 1;
    while ((vripCount++ <= 255 ) &&
           ((optionalPar = params.findParameter(optionalPar, nullptr, "vrip", true)) != nullptr))
    {
        vrrp.addVirtualIP (optionalPar->asIPv4());
    }

    vrrp.compile (params.findParameter ("smac", ownMac)->asMac (), !userDefinedChecksum);

    return (int)vrrp.getAllEthernetPackets(packets);
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
        "eth(dmac=11:22:33:44:55:66, smac=aa:bb:cc:dd:ee:ff, vlan=1, vid=42, prio=3, dsap = 0x12, ssap = 0x34, control = 0x11, payload = 1122)",
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
    nn::Console::PrintDebug("-- " __FILE__ " --\n");
    {
        const char s[] = "  abc \tde_0f  ghi";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = parseTimestamp (s, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (p == &s[2]);
    }
    {
        const char s[] = "1000";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = parseTimestamp (s, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (catched);
        BUG_ON (!p);
    }
    {
        const char s[] = "+1000";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = parseTimestamp (s, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (catched);
        BUG_ON (!p);
    }
    {
        const char s[] = "1000:";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = parseTimestamp (s, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (abs);
        BUG_ON (p == &s[sizeof(s)]-1);
        BUG_ON (t == 1000);
    }
    {
        const char s[] = "+1000:";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = parseTimestamp (s, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (!abs);
        BUG_ON (p == &s[sizeof(s)]-1);
        BUG_ON (t == 1000);
    }
    {
        const char s[] = " 1000 : ";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = parseTimestamp (s, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (abs);
        BUG_ON (p == &s[7]);
        BUG_ON (t == 1000);
    }
    {
        const char s[] = " +1000 : ";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = parseTimestamp (s, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (!abs);
        BUG_ON (p == &s[8]);
        BUG_ON (t == 1000);
    }
    {
        const char s[] = " 1asd000 : ";
        const char* p = nullptr;
        bool catched = false;
        bool abs = false;
        uint64_t t = 0;

        try
        {
            p = parseTimestamp (s, t, abs);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (catched);
        BUG_ON (abs);
        BUG_ON (!p);
        BUG_ON (t==1);
    }
    {
        const char s[] = " abcd(";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (len == 4);
        BUG_ON (!strncmp("abcd", prot, len));
        BUG_ON (*p=='(');
    }
    {
        const char s[] = " abcd (";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (len == 4);
        BUG_ON (!strncmp("abcd", prot, len));
        BUG_ON (*p=='(');
    }
    {
        const char s[] = "abcd(";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (len == 4);
        BUG_ON (!strncmp("abcd", prot, len));
        BUG_ON (*p=='(');
    }
    {
        const char s[] = "abcd( ";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (len == 4);
        BUG_ON (!strncmp("abcd", prot, len));
        BUG_ON (*p=='(');
    }
    {
        const char s[] = " ab42cd-ef(";
        const char* prot;
        size_t len = 0;
        const char* p = nullptr;
        bool catched = false;

        try
        {
            p = parseProtocollIdentifier (s, &prot, &len);
        }
        catch (ParseException& )
        {
            catched = true;
        }
        BUG_ON (!catched);
        BUG_ON (len == 9);
        BUG_ON (!strncmp("ab42cd-ef", prot, len));
        BUG_ON (*p=='(');
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
                p = parseProtocollIdentifier (wrongstrings[n], &prot, &len);
            }
            catch (ParseException& )
            {
                catched = true;
            }
            BUG_ON (catched);
            BUG_ON (!len);
            BUG_ON (!p);
        }
    }


    //TODO fuzzing of parseTimestamp and parseProtocollIdentifier


    uint64_t timestamp;
    bool isAbsolute;
    cMacAddress ownMac("ba:ba:ba:ba:ba:ba");
    cIpAddress ownIPv4;
    std::list <cEthernetPacket> packets;

    ownIPv4.set("10.10.10.10");

    for (unsigned n = 0; n < sizeof(tests)/sizeof(tests[0]); n++)
    {
        nn::Console::PrintDebug("packet %d", n);
        cInstructionParser obj (ownMac, ownIPv4);
        BUG_ON (1 == obj.parse (tests[n].tokens, timestamp, isAbsolute, packets));
        BUG_ON (packets.size () == n + 1);
        BUG_ON (tests[n].packetSize == packets.back().getLength());
        if (memcmp (packets.back().get(), tests[n].packet, tests[n].packetSize))
        {
            const uint8_t* p = packets.back().get();

            for (size_t n = 0; n < packets.back().getLength(); n++)
            {
                printf ("0x%02x, ", (int)*p++);
            }
            printf ("\n");
        }
        BUG_ON (!memcmp (packets.back().get(), tests[n].packet, tests[n].packetSize));
        nn::Console::PrintDebug("\r");
    }
}
#endif /*WITH_UNITTESTS*/
