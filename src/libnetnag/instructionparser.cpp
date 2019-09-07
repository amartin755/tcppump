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


cInstructionParser::cInstructionParser (mac_t ownMac, ipv4_t ownIPv4)
{
	this->ownMac  = ownMac;
	this->ownIPv4 = ownIPv4;
}


cInstructionParser::~cInstructionParser ()
{
}


#ifdef WITH_TIMESTAMP
int cInstructionParser::parse (const char* instruction, cTimeval& timestamp, bool& isAbsolute, cEthernetPacket& packet)
#else
int cInstructionParser::parse (const char* instruction, cEthernetPacket& packet)
#endif
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

		timestamp.set (strtoull (p, &end, 10));
		// check if timestamp is followed by whitespace
		if (!isspace (*end) && *end != '\0')
		{
			throw ParseException ("Invalid timestamp", p);
		}
		p = end;
	}
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
		//TODO find better way for protocol selection
		if (!strncmp ("raw", keyword, keywordEnd - keyword))
			return compileRAW (params, packet);
		if (!strncmp ("eth", keyword, keywordEnd - keyword))
			return compileETH (params, packet);
		if (!strncmp ("arp", keyword, keywordEnd - keyword))
			return compileARP (params, packet);

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
	const cParameter* optionalPar;
	bool hasLlcHeader = false;

	// MAC header
	// default value of source mac is our own mac address
	eth.setMacHeader (params.findParameter ("src", ownMac)->asMac (),
			          params.findParameter ("dest")->asMac ());


	// VLAN tag
	optionalPar = params.findParameter("vid", true);
	if (optionalPar)
	{
		eth.addVlanTag ((int)params.findParameter ("vtype", (uint32_t)1)->asInt8  (1, 2) == 1 ? true : false,
					    (int)optionalPar->asInt16 (0, 0x0fff), // VID
						(int)params.findParameter ("prio", (uint32_t)0)->asInt8  (0, 7),
						(int)params.findParameter ("dei",  (uint32_t)0)->asInt8  (0, 1));
	}

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
	if (hasLlcHeader || (optionalPar = params.findParameter ("ethertype", true)) == NULL)
	{
		eth.setLength ();
	}
	else
	{
		eth.setTypeLength (optionalPar->asInt16 ());
	}


	return eth.getLength();
}


int cInstructionParser::compileARP (cParameterList&, cEthernetPacket&)
{
#if 0
	ethernet_t* e = (ethernet_t*)frame;
	arp_t*      a = (arp_t*)e->payload;

	// default values if not provided
	a->srcMac = ownMac;
	a->srcIp      = ownIPv4;
	a->opcode       = 1;
	memset (&a->dstMac, 0, sizeof (a->dstMac));

	a->hwType       = htons (1);
	a->protType     = htons (ETHERTYPE_IPV4);
	a->hwAddrSize   = sizeof (mac_t);
	a->protAddrSize = sizeof (ipv4_t);
//###
/*	params.getValue (true, "op", a->opcode);
	a->opcode = htons (a->opcode);
	params.getValue (true, "target_mac", a->dstMac);
	params.getValue (true, "sender_mac", a->srcMac);
	params.getValue (false, "target_ip", a->dstIp);
	params.getValue (true, "sender_ip", a->srcIp);
*/
	e->header.typeOrLength = htons (ETHERTYPE_ARP);
	e->header.src = a->srcMac;
	if (a->dstMac.isNull ())
		e->header.dest.set (0xff);
	else
		e->header.dest = a->dstMac;


	return sizeof (arp_t) + sizeof (e->header);
#endif
	return 0;
}


int cInstructionParser::compileIPv4 (cParameterList&, cEthernetPacket&)
{
#if 0
	int i = -1;
	ethernet_t*    e = (ethernet_t*)frame;
	ipv4_header_t* h = (ipv4_header_t*)e->payload;
	char* pEnd;

	// TODO handling of optional parameters

	if (tokenCnt != 11)
	{
		return parseError ("Invalid argument count");
	}

	if (!cTools::macStringToBin (tokens[++i], &e->header.dest))
	{
		return parseError ("Invalid destination mac address", tokens[i]);
	}
	if (!cTools::macStringToBin (tokens[++i], &e->header.src))
	{
		return parseError ("Invalid source mac address", tokens[i]);
	}

	e->header.typeOrLength = htons(ETHERTYPE_IPV4);

	h->vers_ihl = (20 / 4) | (4 << 4);
	h->tos = (uint8_t)strtoul (tokens[++i], &pEnd, 0);
	if (*pEnd != '\0')
	{
		return parseError ("Invalid TOS", tokens[i]);
	}
	h->len = htons (sizeof (*h));
	h->ident = htons ((uint16_t)strtoul (tokens[++i], &pEnd, 0));
	if (*pEnd != '\0')
	{
		return parseError ("Invalid TOS", tokens[i]);
	}

	uint16_t flags = (uint16_t)strtoul (tokens[++i], &pEnd, 0);
	if (*pEnd != '\0' || (flags & ~7))
	{
		return parseError ("Invalid flags", tokens[i]);
	}
	uint16_t offset = (uint16_t)strtoul (tokens[++i], &pEnd, 0);
	if (*pEnd != '\0' || (offset & ~0x1FFF))
	{
		return parseError ("Invalid offset", tokens[i]);
	}
	h->flags_offset = htons (offset | (flags << 13));
	h->ttl = (uint8_t)strtoul (tokens[++i], &pEnd, 0);
	if (*pEnd != '\0')
	{
		return parseError ("Invalid TTL", tokens[i]);
	}
	h->protocol = (uint8_t)strtoul (tokens[++i], &pEnd, 0);
	if (*pEnd != '\0')
	{
		return parseError ("Invalid protocol", tokens[i]);
	}
	h->srcIp = inet_addr (tokens[++i]);
	if (h->srcIp == INADDR_NONE)
	{
		return parseError ("Invalid source IP address", tokens[i]);
	}
	h->dstIp = inet_addr (tokens[++i]);
	if (h->dstIp == INADDR_NONE)
	{
		return parseError ("Invalid destination IP address", tokens[i]);
	}

	int length = MAX_ETHERNET_FRAME_LEN - sizeof (e->header) - sizeof (*h);
	if (!cTools::hexStringToBin (tokens[++i], &length, frame + sizeof (e->header) + sizeof (*h)))
		length = 0;

	h->len = htons (length + sizeof (*h));
	h->chksum = 0;
	h->chksum = htons(cTools::getIPv4Chksum ((uint16_t*)h, sizeof (*h)));

	return length + sizeof (e->header) + sizeof (*h);
#endif
	return 0;
}

