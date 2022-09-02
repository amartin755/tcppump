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


#ifndef IGMP_PACKET_H_
#define IGMP_PACKET_H_

#include <vector>
#include "ipv4packet.hpp"

/*
   V0 (RFC988)

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |           Checksum            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                          Identifier                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Group Address                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   +                         Access Key                            +
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   V1 (RFC1112)

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version| Type  |    Unused     |           Checksum            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Group Address                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   V2 (RFC2236)

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Type     | Max Resp Time |           Checksum            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Group Address                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   V3 (RFC3376)

   Query

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Type = 0x11  | Max Resp Code |           Checksum            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Group Address                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | Resv  |S| QRV |     QQIC      |     Number of Sources (N)     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                       Source Address [1]                      |
   +-                                                             -+
   |                       Source Address [2]                      |
   +-                              .                              -+
   .                               .                               .
   .                               .                               .
   +-                                                             -+
   |                       Source Address [N]                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Report

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Type = 0x22  |    Reserved   |           Checksum            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Reserved            |  Number of Group Records (M)  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   .                                                               .
   .                        Group Record [1]                       .
   .                                                               .
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   .                                                               .
   .                        Group Record [2]                       .
   .                                                               .
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                               .                               |
   .                               .                               .
   |                               .                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   .                                                               .
   .                        Group Record [M]                       .
   .                                                               .
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Group Record
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |  Record Type  |  Aux Data Len |     Number of Sources (N)     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       Multicast Address                       |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       Source Address [1]                      |
    +-                                                             -+
    |                       Source Address [2]                      |
    +-                                                             -+
    .                               .                               .
    .                               .                               .
    .                               .                               .
    +-                                                             -+
    |                       Source Address [N]                      |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    .                                                               .
    .                         Auxiliary Data                        .
    .                                                               .
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/

#pragma pack(1)

typedef struct
{
    uint8_t  version_type;
    uint8_t  unused;
    uint16_t checksum;
    struct in_addr groupAddress;

}igmpv1_packet_t;

typedef struct
{
    uint8_t  type;
    uint8_t  maxRespTime;
    uint16_t checksum;
    struct in_addr groupAddress;

}igmpv2_packet_t;

typedef struct
{
    uint8_t  type;
    uint8_t  maxRespCode;
    uint16_t checksum;
    struct in_addr groupAddress;

    uint8_t flags;
    uint8_t qqic;
    uint16_t numberOfSources;

    //    struct in_addr sources [numberOfSources]; see member v3sourceAddresses

    void setS (bool s)
    {
        flags &= 0x08;
        flags |= (((uint8_t)s) << 3);
    }
    void setQRV (unsigned qrv)
    {
        flags &= 0xf8;
        flags |= (qrv & 7);
    }
}igmpv3_query_t;

typedef struct
{
    uint8_t  type;
    uint8_t  reserved1;
    uint16_t checksum;
    uint16_t reserved2;
    uint16_t numbOfGroupRecords;

    //    struct igmpv3_group_record_t groupRecords [numbOfGroupRecords];

}igmpv3_report_t;

typedef struct
{
    uint8_t  type;
    uint8_t  auxDataLen;
    uint16_t numberOfSources;
    struct in_addr groupAddress;
    //    struct in_addr sources [numberOfSources];

}igmpv3_group_record_t;

#pragma pack()

class cIgmpPacket : public cIPv4Packet
{
public:
    cIgmpPacket();
    void v12compile (uint8_t type, uint8_t time, const cIPv4& group);
    void compileGeneralQuery (bool v3, double maxRespCode, bool s, unsigned qrv, double qqic);
    void compileGroupQuery (bool v3, double maxRespCode, bool s, unsigned qrv, double qqic, const cIPv4& group);
    void compileReport (const cIPv4& group);
    void v2compileLeaveGroup (const cIPv4& group);

    void v3addSource (const cIPv4& source);

private:
    void v3compileGeneralQuery (double maxRespCode, bool s, unsigned qrv, double qqic);
    void v3compileGroupQuery (double maxRespCode, bool s, unsigned qrv, double qqic, const cIPv4& group);
    void setIpHeaderOptions (void);
    uint8_t floatToTime (double d) const;
    std::vector<struct in_addr> v3sourceAddresses;
};

#endif /* IGMP_PACKET_H_ */
