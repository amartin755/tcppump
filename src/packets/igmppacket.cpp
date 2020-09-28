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
#include "inet.h"
#include "igmppacket.hpp"
#include "inetchecksum.hpp"



cIgmpPacket::cIgmpPacket ()
{
}

void cIgmpPacket::compileGeneralQuery (uint8_t time)
{
    setDestination (cIpAddress ("224.0.0.1"));
    compile (0x11, time, cIpAddress ());
}

void cIgmpPacket::compileGroupQuery (uint8_t time, const cIpAddress& group)
{
    setDestination (group);
    compile (0x11, time, group);
}

void cIgmpPacket::compileReport (const cIpAddress& group)
{
    setDestination (group);
    compile (0x16, 0, group);
}

void cIgmpPacket::compileLeaveGroup (const cIpAddress& group)
{
    setDestination (cIpAddress ("224.0.0.2"));
    compile (0x17, 0, group);
}

void cIgmpPacket::compile (uint8_t type, uint8_t time, const cIpAddress& group)
{
    // set IPv4 header flags
    setTimeToLive (1);
    setDSCP (48);
    setDontFragment (true);

    // add RouterOption to ip header
    addRouterAlertOption ();

    igmpv2_packet_t igmp;
    std::memset (&igmp, 0, sizeof (igmp));

    igmp.type = type;
    igmp.maxRespTime = time;
    igmp.groupAddress = group.get();

    igmp.checksum = cInetChecksum::rfc1071 (&igmp, sizeof (igmp));

    cIPv4Packet::compile (PROTO_IGMP, (const uint8_t*)&igmp, sizeof (igmp), nullptr, 0, true);
}


