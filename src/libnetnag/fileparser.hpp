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


#ifndef FILEPARSER_HPP_
#define FILEPARSER_HPP_

#include <cstdint>
#include <cstdio>
#include <list>

#include "protocoltypes.hpp"
#include "ethernetpacket.hpp"
#include "timeval.hpp"

const int PARSE_ERROR = -100;

class cParameterList;

class cFileParser
{
public:
	cFileParser ();
	~cFileParser ();
	void init (FILE* fp, const cTimeval& defaultDelay, mac_t ownMac, ipv4_t ownIPv4);
	int parse (cTimeval&, bool&, std::list <cEthernetPacket> &packets);
	const char* getLastError ();


private:
	int parseError (const char* errmsg, const char* pos = NULL);

	char*  instructionBuffer;
	int    instructionBufferSize;

	cTimeval    delay;
	mac_t  ownMac;
	ipv4_t ownIPv4;
	FILE*  fp;

	char lastError[1024];
	unsigned lineNbr;
};

#endif /* FILEPARSER_HPP_ */
