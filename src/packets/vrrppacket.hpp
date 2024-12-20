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

#ifndef VRRP_PACKET_H_
#define VRRP_PACKET_H_

#include <vector>
#include "ipv4packet.hpp"

/*
   V2 (RFC3768)

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version| Type  | Virtual Rtr ID|   Priority    | Count IP Addrs|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   Auth Type   |   Adver Int   |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         IP Address (1)                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                            .                                  |
   |                            .                                  |
   |                            .                                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         IP Address (n)                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                     Authentication Data (1)                   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                     Authentication Data (2)                   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   V3 (RFC5798)

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version| Type  | Virtual Rtr ID|   Priority    |Count IPvX Addr|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |(rsvd) |     Max Adver Int     |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   +                                                               +
   |                       IPvX Address(es)                        |
   +                                                               +
   +                                                               +
   +                                                               +
   +                                                               +
   |                                                               |
   +                                                               +
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/

typedef struct
{
    uint8_t vers_type; // version (bit 0 - 3) | type (bit 4 - 7)
    uint8_t vrid;
    uint8_t prio;         // 0 - 255
    uint8_t countIpAddr;  // 1 - 255
    union
    {
        struct
        {
            uint8_t authType;
            uint8_t adverInt;
        } v2;
        struct
        {
            uint16_t maxAdverInt; // only 12 bits
        } v3;

    };
    uint16_t chksum;
//    struct in_addr ipAddresses[countIpAddr];


    void setVersion (int version)
    {
        vers_type = ((version & 0x0F) << 4) | (vers_type & 0x0F);
    }
    void setType (int type)
    {
        vers_type = (type & 0x0F) | (vers_type & 0xF0);
    }
    int getVersion ()
    {
        return (int)((vers_type >> 4) & 0x0F);
    }
    int getType ()
    {
        return (int)(vers_type & 0x0F);
    }

}vrrp_header_t;
static_assert (sizeof (vrrp_header_t) == 8, "vrrp_header_t is not packed");


class cVrrpPacket : public cIPv4Packet
{
public:
    cVrrpPacket();
    void setVersion (int version);
    void setVRID (uint8_t vrid);
    void setPrio (uint8_t prio);
    void setType (uint8_t type);
    void setInterval (uint16_t interval);
    void setChecksum (uint16_t checksum);
    void addVirtualIP (const cIPv4& vip);
    void compile (bool calcChecksum);



private:
    vrrp_header_t header;
    std::vector<struct in_addr> vrIPs;
};

#endif /* VRRP_PACKET_H_ */
