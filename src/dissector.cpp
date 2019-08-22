/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2019 Andreas Martin (netnag@mailbox.org)
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

#include <cassert>

#include "libnetnag/console.hpp"


using namespace nn;

cDissector::cDissector (cEthernetPacket& p)
	: packet (p.get()), packetLength(p.getLength())
{
}

cDissector::~cDissector()
{
	// TODO Auto-generated destructor stub
}

//FIXME this code is only for proof-of-concept. It's a total mess and must be cleaned up!
bool cDissector::dissect () const
{
	bool isMalformed = false;
	mac_header_t* header = (mac_header_t*)packet;

	if (sizeof (mac_header_t) > packetLength)
	{
		isMalformed = true;
	}
	else
	{
		nn::Console::Print("%02x:%02x:%02x:%02x:%02x:%02x > %02x:%02x:%02x:%02x:%02x:%02x, ",
				header->src.a, header->src. b, header->src.c, header->src.d, header->src.e, header->src.f,
				header->dest.a, header->dest. b, header->dest.c, header->dest.d, header->dest.e, header->dest.f);

		uint16_t* typeLength = &header->ethertypeLength;

		for (vlan_t* tag = (vlan_t*)&header->ethertypeLength; tag->isVlan() && !isMalformed; tag++, typeLength = (uint16_t*)tag)
		{
			tag->isCVlan() ? nn::Console::Print ("C-VLAN ") : nn::Console::Print ("P-VLAN ");
			if (isWithinPacket (tag + 1, sizeof (tag->tpid)))
			{
				nn::Console::Print ("id %u prio %u DEI %u, ", tag->getId(), tag->getPrio(), tag->getDEI());
			}
			else
			{
				isMalformed = true;
			}
		}

		if (!isMalformed)
		{
			uint16_t type = ntohs(*typeLength);
			switch (type)
			{
			case ETHERTYPE_IPV4:
				nn::Console::Print("IPv4, ");
				break;
			case ETHERTYPE_ARP:
				nn::Console::Print("ARP, ");
				break;
			case ETHERTYPE_PN:
				nn::Console::Print("PN, ");
				break;
			default:
				if (type <= cEthernetPacket::MAX_ETHERNET_PAYLOAD)
				{
					size_t llcHeaderLength;

					nn::Console::Print("LLC (length %d) ", type);


					size_t len = packetLength - ((uint8_t*)(typeLength + 1) - packet);

					if (len != type)
					{
						nn::Console::Print("XXXX ");
						isMalformed = true;
					}

					if (len >= (sizeof (llc_t) - 1))
					{
						llc_t* llc = (llc_t*)(typeLength + 1);
						nn::Console::Print("DSAP 0x%02x, SSAP 0x%02x, ", llc->dsap, llc->ssap);
						if ((llc->control.c8 & 0x03) == 3)
						{
							llcHeaderLength = sizeof (llc_t) - 1;
							nn::Console::Print("Control 0x%02x, ", llc->control.c8);
						}
						else
						{
							llcHeaderLength = sizeof (llc_t);
							if (len >= llcHeaderLength)
								nn::Console::Print("Control 0x%04x, ", ntohs(llc->control.c16));
							else
								isMalformed = true;
						}

						// SNAP header
						if (!isMalformed && llc->dsap == 0xaa && llc->ssap == 0xaa)
						{
							snap_t* snap = (snap_t*)((uint8_t*)llc + llcHeaderLength);
							if (len >= (llcHeaderLength + sizeof(snap_t)))
							{
								nn::Console::Print("SNAP OUI 0x%02x%02x%02x protocol 0x%04x, ", snap->oui.a, snap->oui.b, snap->oui.c, ntohs(snap->protocol));
							}
							else
								isMalformed = true;
						}
					}
					else
					{
						isMalformed = true;
					}
				}
				else
					nn::Console::Print("unknown (%04x), ", type);
			}
		}
	}
	if (isMalformed)
	{
		nn::Console::Print ("malformed packet, length %u", packetLength);
	}
	nn::Console::Print("\n");
	dump (packet, packetLength);

	return !isMalformed;
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
	default:
		return "unknown";

	}
	assert ("unreachable code" == 0);
	return "";
}

bool cDissector::isWithinPacket (const void* p, size_t size) const
{
	assert (p >= packet);
	assert (size > 0);
	return ((uint8_t*)p + size) <= (packet + packetLength);
}

