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

#ifndef DISSECTOR_H_
#define DISSECTOR_H_

#include <cstddef> // size_t

#include "libnetnag/ethernetpacket.hpp"

class cDissector
{
public:
	cDissector (cEthernetPacket& packet);
	~cDissector();

	bool dissect () const;

private:
	void dump (const void* p, size_t length) const;
	const char* ethertypeToString (uint16_t ethertype) const;
	bool isWithinPacket (const void* p, size_t size) const;

	const uint8_t*  packet;
	const size_t    packetLength;
};

#endif /* DISSECTOR_H_ */
