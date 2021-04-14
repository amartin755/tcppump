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


#include "dissector.hpp"

#include <string>

#include "bug.hpp"
#include "inet.h"
#include "console.hpp"
#include "ipv4packet.hpp"


cDissector::cDissector (const cEthernetPacket& p)
    : packet (p.get()), packetLength(p.getLength())
{
}

cDissector::cDissector (const uint8_t* p, size_t l)
: packet (p), packetLength(l)
{
}

cDissector::~cDissector()
{
    // TODO Auto-generated destructor stub
}


bool cDissector::dissect () const
{
    mac_header_t* header = (mac_header_t*)packet;
    const void* payload = NULL;
    bool ok = true;

    try
    {
        if (sizeof (mac_header_t) > packetLength)
        {
            throw "Packet too short (< 14 bytes)";
        }
        // first, we print the mac header
        Console::Print("%02x:%02x:%02x:%02x:%02x:%02x > %02x:%02x:%02x:%02x:%02x:%02x (packet length %u)\n  ",
                header->src.mac[0], header->src.mac[1], header->src.mac[2], header->src.mac[3], header->src.mac[4], header->src.mac[5],
                header->dest.mac[0], header->dest.mac[1], header->dest.mac[2], header->dest.mac[3], header->dest.mac[4], header->dest.mac[5],
                (unsigned)packetLength);

        // ... then walk through possible existing VLAN and LLC/SNAP headers
        const uint16_t* typeLength = (const uint16_t*)dissectVLAN (&header->ethertypeLength);
                        typeLength = (const uint16_t*)dissectLLC (typeLength);

        // ... now it's time for processing some known L3 protocols
        if (typeLength)
        {
            uint16_t type = ntohs(*typeLength);
            switch (type)
            {
            case ETHERTYPE_IPV4:
                payload = dissectIPv4 (typeLength + 1);
                break;
            case ETHERTYPE_ARP:
                payload = dissectARP (typeLength + 1);
                break;
            case ETHERTYPE_PN:
                payload = dissectPN (typeLength + 1);
                break;
            default:
                payload = unknown (typeLength);
            }
        }
        // Finally, all upper layer protocols are just 'payload'
        if (payload)
            Console::Print("Payload %u bytes\n ", unsigned(packet + packetLength - (uint8_t*)payload));
    }
    catch (const char *malformed)    // malformed packet?
    {
        Console::Print ("%s\n  ", malformed);
        ok = false;
    }
    catch (...)
    {
        BUG ("unexpected exception");
    }
    Console::Print("\n");
    dump (packet, packetLength);
    Console::PrintVerbose("\n");

    return ok;
}


const void* cDissector::dissectVLAN (const void * pTypeLength) const
{
    const vlan_t* tag = (const vlan_t*)pTypeLength;

    if (!tag->isVlan()) return pTypeLength;

    tag->isCVlan() ? Console::Print ("C-VLAN ") : Console::Print ("P-VLAN ");

    if (isWithinPacket (tag + 1, sizeof (tag->tpid)))
    {
        Console::Print ("id %u, prio %u, DEI %u\n  ", tag->getId(), tag->getPrio(), tag->getDEI());
        return dissectVLAN (tag + 1);
    }

    throw "malformed packet";
    return NULL;
}


const void* cDissector::dissectLLC (const void * pTypeLength) const
{
    uint16_t typeLength = ntohs(*(uint16_t*)pTypeLength);

    if (typeLength > cEthernetPacket::MAX_ETHERNET_PAYLOAD)
        return pTypeLength;


    Console::Print("LLC (length %d) ", typeLength);


    size_t payloadLen = packetLength - ((uint8_t*)((uint16_t*)pTypeLength + 1) - packet);

    if (payloadLen != typeLength)
    {
        throw "malformed packet XXX";
    }

    if (payloadLen >= (sizeof (llc_t) - 1))
    {
        size_t llcHeaderLength;
        llc_t* llc = (llc_t*)((uint16_t*)pTypeLength + 1);
        Console::Print("DSAP 0x%02x, SSAP 0x%02x, ", llc->dsap, llc->ssap);
        if ((llc->control.c8 & 0x03) == 3)
        {
            llcHeaderLength = sizeof (llc_t) - 1;
            Console::Print("Control 0x%02x\n  ", llc->control.c8);
        }
        else
        {
            llcHeaderLength = sizeof (llc_t);
            if (payloadLen >= llcHeaderLength)
                Console::Print("Control 0x%04x\n  ", ntohs(llc->control.c16));
            else
                throw "malformed packet";
        }

        // SNAP extention header
        if (llc->dsap == 0xaa && llc->ssap == 0xaa)
        {
            snap_t* snap = (snap_t*)((uint8_t*)llc + llcHeaderLength);
            if (payloadLen >= (llcHeaderLength + sizeof(snap_t)))
            {
                Console::Print("SNAP OUI 0x%02x%02x%02x protocol 0x%04x\n  ", snap->oui.a, snap->oui.b, snap->oui.c, ntohs(snap->protocol));
                return &snap->protocol;
            }
            else
                throw "malformed packet";
        }
    }
    else
    {
        throw "malformed packet";
    }
    return NULL;
}


const void* cDissector::dissectARP (const void * p) const
{
    Console::Print("ARP\n  ");
    return p;
}


const void* cDissector::dissectIPv4 (const void * p) const
{
    //FIXME only a hack to show at least the ip addresses and L4 protocol
    Console::Print("IPv4 ");

    const ipv4_header_t* header = (const ipv4_header_t*)p;

    if (!isWithinPacket (header, sizeof (header)))
        throw "malformed packet";

    std::string src, dst;
    cIpAddress(header->srcIp).get (src);
    cIpAddress(header->dstIp).get (dst);

    Console::Print ("%s > %s, ", src.c_str(), dst.c_str());

    switch (header->protocol)
    {
    case 1:
        Console::Print ("ICMP");
        break;
    case 2:
        Console::Print ("IGMP");
        break;
    case 17:
        Console::Print ("UDP");
        break;
    case 6:
        Console::Print ("TCP");
        break;
    case 50:
        Console::Print ("ESP");
        break;
    case 115:
        Console::Print ("L2TP");
        break;
    default:
        Console::Print ("Protocol %d", header->protocol);
    }

    Console::Print ("\n  ");

    return header + 1;
}


const void* cDissector::dissectPN (const void * p) const
{
    Console::Print("PN\n  ");
    return p;
}


const void* cDissector::unknown (const void * p) const
{
    uint16_t ethertype = ntohs(*(uint16_t*)p);

    const char* str = ethertypeToString (ethertype);
    if (str)
        Console::Print("%s\n  ", str);
    else
        Console::Print("Unknown ethertype 0x%04x\n  ", ethertype);

    return (uint16_t*)p + 1;
}


// based on https://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
void cDissector::dump (const void* p, size_t length) const
{
    unsigned i;
    unsigned char buff[17];
    const uint8_t *pc = (const uint8_t*)p;


    if (length == 0)
    {
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < length; i++)
    {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0)
        {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                Console::PrintVerbose ("  %s\n", buff);

            // Output the offset.
            Console::PrintVerbose ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        Console::PrintVerbose (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0)
    {
        Console::PrintVerbose ("   ");
        i++;
    }

    // And print the final ASCII bit.
    Console::PrintVerbose ("  %s\n", buff);
}


const char* cDissector::ethertypeToString (uint16_t ethertype) const
{
    // TODO replace with hash-table
    switch (ethertype)
    {
    case ETHERTYPE_IPV4:
        return "IPv4";
    case ETHERTYPE_ARP:
        return "ARP";
    case ETHERTYPE_PN:
        return "PN";
    case ETHERTYPE_CVLAN:
        return "C-VLAN";
    case ETHERTYPE_SVLAN:
        return "P-VLAN";
    case 0x0842:
        return "Wake-on-LAN";
    case 0x22F3:
        return "TRILL";
    case 0x22EA:
        return "SRP";
    case 0x8035:
        return "RARP";
    case 0x8102:
        return "SLPP";
    case 0x8137:
        return "IPX";
    case 0x86DD:
        return "IPv6";
    case 0x8808:
        return "Ethernet Flow Control";
    case 0x8863:
        return "PPPoE Discovery";
    case 0x8864:
        return "PPPoE Session";
    case 0x88A4:
        return "EtherCAT";
    case 0x88CC:
        return "LLDP";
    case 0x88E3:
        return "MRP";
    case 0x88E5:
        return "MACsec";
    case 0x88F5:
        return "MVRP";
    case 0x88F6:
        return "MMRP";
    case 0x880B:
        return "PPP";

    }
    return NULL;
}


bool cDissector::isWithinPacket (const void* p, size_t size) const
{
    BUG_ON (p < packet);
    BUG_ON (size <= 0);
    return ((uint8_t*)p + size) <= (packet + packetLength);
}

