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


#ifndef DISSECTOR_H_
#define DISSECTOR_H_

#include <cstddef> // size_t

#include "ethernetpacket.hpp"

class cDissector
{
public:
    cDissector (const cEthernetPacket& packet);
    cDissector (const uint8_t* packet, size_t len);
    ~cDissector();

    bool dissect () const;

private:
    const void* dissectVLAN (const void *) const;
    const void* dissectLLC  (const void *) const;
    const void* dissectARP  (const void *) const;
    const void* dissectIPv4 (const void *) const;
    const void* dissectPN   (const void *) const;
    const void* unknown   (const void *) const;
    void dump (const void* p, size_t length) const;
    const char* ethertypeToString (uint16_t ethertype) const;
    bool isWithinPacket (const void* p, size_t size) const;

    const uint8_t*  packet;
    const size_t    packetLength;
};

#endif /* DISSECTOR_H_ */
