// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2025 Andreas Martin (netnag@mailbox.org)
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
#include "grepacket.hpp"
#include "bug.hpp"
#include "inetchecksum.hpp"


cGrePacket::cGrePacket (bool isIPv6)
: cIPPacket (isIPv6), hasChecksum (false), hasKey (false), hasSeq (false), checksum (0), key (0), seq  (0)
{
    std::memset (&basicHeader, 0, sizeof(basicHeader));
}

void cGrePacket::compile (const uint8_t* payload, size_t len, bool calcChecksum)
{
    uint32_t header[sizeof (basicHeader) / 4 + 1 + 1 + 1]; // + checksum + key + sequence
    unsigned headerLen = sizeof (basicHeader);

    if (calcChecksum)
        setChecksum (0);

    uint32_t* p = header;
    std::memcpy(p, &basicHeader, sizeof (basicHeader));
    p++;
    if (hasChecksum)
    {
        *p++ = (uint32_t)htons(checksum);
        headerLen += 4;
    }
    if (hasKey)
    {
        *p++ = htonl (key);
        headerLen += 4;
    }
    if (hasSeq)
    {
        *p++ = htonl (seq);
        headerLen += 4;
    }

    cIPPacket::compile (PROTO_GRE, (const uint8_t*)&header, headerLen, payload, len);
    if (calcChecksum)
    {
        p = header + 1;
        *p = cInetChecksum::rfc1071 (&header, headerLen, payload, len);
        cIPPacket::updateL4Header ((const uint8_t*)&header, headerLen);
    }
}

void cGrePacket::setProtocolType (uint16_t proto)
{
    basicHeader.protocol = htons (proto);
}

void cGrePacket::setKey (uint32_t k)
{
    this->key = k;
    this->hasKey = true;
    basicHeader.setKeyFlag();
}

void cGrePacket::setSequence (uint32_t s)
{
    this->seq = s;
    this->hasSeq = true;
    basicHeader.setSeqFlag();
}

void cGrePacket::setChecksum (uint16_t chk)
{
    this->checksum = chk;
    this->hasChecksum = true;
    basicHeader.setChecksumFlag();
}


#ifdef WITH_UNITTESTS
#include "console.hpp"

void cGrePacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");
}
#endif
