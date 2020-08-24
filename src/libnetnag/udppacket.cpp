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


#include <cassert>
#include <cstring>

#include "inet.h"
#include "udppacket.hpp"



cUdpPacket::cUdpPacket ()
{
	this->setTypeLength (ETHERTYPE_IPV4);
}

void cUdpPacket::setPayload (const char* payload, size_t len)
{
}

void cUdpPacket::setSourcePort (uint16_t port)
{

}

void cUdpPacket::setDestinationPort (uint16_t port)
{

}

void cUdpPacket::setChecksum (uint16_t checksum)
{

}

#ifdef WITH_UNITTESTS
#include "console.hpp"

void cUdpPacket::unitTest ()
{
	nn::Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
