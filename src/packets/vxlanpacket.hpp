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


#ifndef VXLAN_PACKET_H_
#define VXLAN_PACKET_H_

#include <cstdint>

#include "udppacket.hpp"

/*
VXLAN Header:
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |R|R|R|R|I|R|R|R|            Reserved                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                VXLAN Network Identifier (VNI) |   Reserved    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
typedef struct
{
    uint8_t  flags;
    uint8_t  reserved[3];
    uint32_t vni; // 24 bits only

}vxlan_header_t;
static_assert (sizeof (vxlan_header_t) == 8, "vxlan_header_t is not packed");


class cVxlanPacket : public cUdpPacket
{
public:
    cVxlanPacket ();

    void compile (const uint8_t* payload, size_t len);
    void setVni (uint32_t vni);


#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    vxlan_header_t header;
};


#endif /* VXLAN_PACKET_H_ */
