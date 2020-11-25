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

#include "bug.hpp"
#include "inetchecksum.hpp"
#include "icmppacket.hpp"



cIcmpPacket::cIcmpPacket ()
{
}

void cIcmpPacket::compileRaw (uint8_t type, uint8_t code, const uint8_t* payload, size_t len)
{
    icmp_header_t header;

    header.type = type;
    header.code = code;
    header.checksum = 0;
    header.checksum = cInetChecksum::rfc1071((const uint16_t*)&header, sizeof(header), payload, len);

    cIPv4Packet::compile (PROTO_ICMP, (const uint8_t*)&header, sizeof (header), payload, len);
}

void cIcmpPacket::compileRaw (uint8_t type, uint8_t code, uint16_t checksum, const uint8_t* payload, size_t len)
{
    icmp_header_t header;

    header.type = type;
    header.code = code;
    header.checksum = checksum;

    cIPv4Packet::compile (PROTO_ICMP, (const uint8_t*)&header, sizeof (header), payload, len);
}

#ifdef WITH_UNITTESTS
#include "console.hpp"

void cIcmpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
