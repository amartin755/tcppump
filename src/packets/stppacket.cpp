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

#include <cstring>

#include "bugon.h"
#include "stppacket.hpp"


cStpPacket::cStpPacket ()
{
}

void cStpPacket::compile (const cMacAddress& srcMac)
{
    // set mac header destination multicast mac address 01:80:C2:00:00:0
    setMacHeader(srcMac, cMacAddress(1, 0x80, 0xc2, 0, 0, 0));

    addLlcHeader (0x42, 0x42, 3);

	bpdu_t bpdu;
	std::memset (&bpdu, 0, sizeof (bpdu));

	bpdu.root.set (8, 0, srcMac);
	bpdu.rootPathCost = htonl (4);
	bpdu.bridge.set (8, 0, srcMac);
	bpdu.setPortId (0, 1);
	bpdu.messageAge = htons (0);
	bpdu.maxAge = htons (20);
	bpdu.forwardDelay = htons (15);
	bpdu.helloTime = htons (123);
	setPayload ((uint8_t*)&bpdu, sizeof (bpdu));
	setLength ();
}

