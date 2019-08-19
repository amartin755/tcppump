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


#ifndef INSTRUCTIONSPARSER_HPP_
#define INSTRUCTIONSPARSER_HPP_

#include <cstdint>
#include <cstdio>

#include "protocoltypes.hpp"
#include "ethernetpacket.hpp"
#ifdef WITH_TIMESTAMP
#include "timeval.hpp"
#endif

class cParameterList;

class cInstructionParser
{
public:
	cInstructionParser (mac_t ownMac, ipv4_t ownIPv4);
	~cInstructionParser ();
#ifdef WITH_TIMESTAMP
	int parse (const char* instruction, cTimeval& timestamp, bool& isAbsolute, cEthernetPacket& packet);
#else
	int parse (const char* instruction, cEthernetPacket& packet);
#endif

private:
	int compileRAW (cParameterList& params, cEthernetPacket& packet);
	int compileETH (cParameterList& params, cEthernetPacket& packet);
	int compileARP (cParameterList& params, cEthernetPacket& packet);
	int compileSNAP (cParameterList& params, cEthernetPacket& packet);
	int compileIPv4 (cParameterList& params, cEthernetPacket& packet);

	mac_t  ownMac;
	ipv4_t ownIPv4;
};

class ParseException
{
public:
	ParseException (const char* msg, const char* val)
	{
		this->msg = msg;
		this->val = val;
	}

	const char* what ()
	{
		return msg;
	}

	const char* value ()
	{
		return val;
	}

private:
	const char* msg;
	const char* val;
};


#endif /* INSTRUCTIONSPARSER_HPP_ */
