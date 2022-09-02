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
#include "igmppacket.hpp"

#include "bug.hpp"
#include "inetchecksum.hpp"



cIgmpPacket::cIgmpPacket ()
{
}

void cIgmpPacket::compileGeneralQuery (bool v3, double maxRespCode, bool s, unsigned qrv, double qqic)
{
    maxRespCode *= 10;
    setDestination (cIPv4 ("224.0.0.1"));
    if (v3)
        v3compileGeneralQuery (maxRespCode, s, qrv, qqic);
    else
        v12compile (0x11, uint8_t(maxRespCode), cIPv4 ());
}

void cIgmpPacket::compileGroupQuery (bool v3, double maxRespCode, bool s, unsigned qrv, double qqic, const cIPv4& group)
{
    maxRespCode *= 10;
    setDestination (group);
    if (v3)
        v3compileGroupQuery (maxRespCode, s, qrv, qqic, group);
    else
        v12compile (0x11, uint8_t(maxRespCode), group);
}

void cIgmpPacket::compileReport (const cIPv4& group)
{
    setDestination (group);
    v12compile (0x16, 0, group);
}

void cIgmpPacket::v2compileLeaveGroup (const cIPv4& group)
{
    setDestination (cIPv4 ("224.0.0.2"));
    v12compile (0x17, 0, group);
}

void cIgmpPacket::v12compile (uint8_t type, uint8_t time, const cIPv4& group)
{
    setIpHeaderOptions ();

    igmpv2_packet_t igmp;
    std::memset (&igmp, 0, sizeof (igmp));

    igmp.type = type;
    igmp.maxRespTime = time;
    igmp.groupAddress = group.get();

    igmp.checksum = cInetChecksum::rfc1071 (&igmp, sizeof (igmp));

    cIPv4Packet::compile (PROTO_IGMP, (const uint8_t*)&igmp, sizeof (igmp), nullptr, 0);
}

void cIgmpPacket::v3compileGeneralQuery (double maxRespCode, bool s, unsigned qrv, double qqic)
{
    v3sourceAddresses.clear ();
    v3compileGroupQuery (maxRespCode, s, qrv, qqic, cIPv4());
}

void cIgmpPacket::v3compileGroupQuery (double maxRespCode, bool s, unsigned qrv, double qqic, const cIPv4& group)
{
    setIpHeaderOptions ();

    igmpv3_query_t igmp;
    std::memset (&igmp, 0, sizeof (igmp));

    igmp.type         = 0x11;
    igmp.maxRespCode  = floatToTime (maxRespCode); //float to int
    igmp.qqic         = floatToTime (qqic);  //float to int
    igmp.groupAddress = group.get();
    igmp.setS (s);
    igmp.setQRV (qrv);
    igmp.numberOfSources = htons((uint16_t)v3sourceAddresses.size());

    if (igmp.numberOfSources)
    {
        igmp.checksum = cInetChecksum::rfc1071(&igmp, sizeof (igmp),
                (const uint8_t*)v3sourceAddresses.data(), v3sourceAddresses.size() * sizeof(struct in_addr));
        cIPv4Packet::compile (PROTO_IGMP, (const uint8_t*)&igmp, sizeof (igmp),
                (const uint8_t*)v3sourceAddresses.data(), v3sourceAddresses.size() * sizeof(struct in_addr));
    }
    else
    {
        igmp.checksum = cInetChecksum::rfc1071 (&igmp, sizeof (igmp));
        cIPv4Packet::compile (PROTO_IGMP, (const uint8_t*)&igmp, sizeof (igmp), nullptr, 0);
    }
}

void cIgmpPacket::v3addSource (const cIPv4& source)
{
    v3sourceAddresses.push_back(source.get());
}

void cIgmpPacket::setIpHeaderOptions (void)
{
    // set IPv4 header flags
    setTimeToLive (1);
    setDSCP (48);
    setDontFragment (true);

    // add RouterOption to ip header
    addRouterAlertOption ();
}

uint8_t cIgmpPacket::floatToTime (double d) const
{
    if (d < 128)
    {
        return (uint8_t) d;
    }
    else if (d >= 31744)
    {
        return 255;
    }
    else
    {
        unsigned exp = 0;
        unsigned mant = (unsigned)d;

        mant >>= 3;
        while (mant > 31)
        {
            exp++;
            mant >>= 1;
        }
        return 0x80 | ((exp & 7) << 4) | (mant & 0x0f);
    }
}
