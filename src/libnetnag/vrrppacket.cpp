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
#include "vrrppacket.hpp"



cVrrpPacket::cVrrpPacket ()
{
    std::memset (&header, 0, sizeof (header));
}

void cVrrpPacket::setVersion (int version)
{

}

void cVrrpPacket::setVRID (uint8_t vrid)
{

}

void cVrrpPacket::setPrio (uint8_t prio)
{

}

void cVrrpPacket::setType (uint8_t type)
{

}

void cVrrpPacket::setInterval (uint16_t interval)
{

}

void cVrrpPacket::setChecksum (uint16_t checksum)
{

}

void cVrrpPacket::addVirtualIP (const cIpAddress& vip)
{

}

void cVrrpPacket::compile (const cMacAddress& srcMac)
{
    // TODO
    // ttl must be set to 255!!!
    // mac header with virtual mac
    // destination ip = 224.0.0.18

    // calculate checksum (do not overwrite user provided checksum!)
}


