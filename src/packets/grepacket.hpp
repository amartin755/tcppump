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


#ifndef GRE_PACKET_H_
#define GRE_PACKET_H_

#include <cstdint>
#include <vector>
#include "ippacket.hpp"


struct gre_basic_header_t
{
    uint8_t  flags;
    uint8_t  version;
    uint16_t protocol;
    bool hasChecksum (void)
    {
        return !!(flags & 0x80);
    }
    bool hasKey (void)
    {
        return !!(flags & 0x20);
    }
    bool hasSequence (void)
    {
        return !!(flags & 0x10);
    }
    void setChecksumFlag (void)
    {
        flags |= 0x80;
    }
    void setKeyFlag (void)
    {
        flags |= 0x20;
    }
    void setSeqFlag (void)
    {
        flags |= 0x10;
    }
};
static_assert (sizeof (gre_basic_header_t) == 4, "gre_basic_header_t is not packed");


class cGrePacket : public cIPPacket
{
public:
    cGrePacket (bool isIPv6);

    void compile (const uint8_t* payload, size_t len, bool calcChecksum);
    void setProtocolType (uint16_t proto);
    void setKey (uint32_t key);
    void setSequence (uint32_t seq);
    void setChecksum (uint16_t checksum);


#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:

    gre_basic_header_t basicHeader;
    bool hasChecksum;
    bool hasKey;
    bool hasSeq;
    uint16_t checksum;
    uint32_t key;
    uint32_t seq;
};


#endif /* GRE_PACKET_H_ */
