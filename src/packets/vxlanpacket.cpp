// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2021 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
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

#include "inet.h"
#include "vxlanpacket.hpp"
#include "bug.hpp"
#include "inetchecksum.hpp"


cVxlanPacket::cVxlanPacket (bool isIPv6) : cUdpPacket (isIPv6)
{
    std::memset (&header, 0, sizeof(header));
    header.flags = 0x08;
}

void cVxlanPacket::compile (const uint8_t* payload, size_t len)
{
    uint8_t* packet = new uint8_t[len + sizeof (header)];
    std::memcpy(packet, &header, sizeof (header));
    std::memcpy(packet + sizeof (header), payload, len);
    cUdpPacket::compile (packet, len + sizeof (header));
    delete[] packet;
}

void cVxlanPacket::setVni (uint32_t vni)
{
    vni = vni & 0x00ffffff;
    header.vni = htonl(vni << 8);
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cVxlanPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
