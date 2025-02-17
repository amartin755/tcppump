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
#include "vrrppacket.hpp"

#include "bug.hpp"
#include "inetchecksum.hpp"



cVrrpPacket::cVrrpPacket ()
{
    std::memset (&header, 0, sizeof (header));
    vrIPs.reserve(16); // set initial capacity to avoid reallocations in typical use cases
}

void cVrrpPacket::setVersion (int version)
{
    header.setVersion(version);
}

void cVrrpPacket::setVRID (uint8_t vrid)
{
    header.vrid = vrid;
}

void cVrrpPacket::setPrio (uint8_t prio)
{
    header.prio = prio;
}

void cVrrpPacket::setType (uint8_t type)
{
    header.setType((int)type);
}

void cVrrpPacket::setInterval (uint16_t interval)
{
    if (header.getVersion() == 2)
        header.v2v3.v2.adverInt = (uint8_t)interval;
    else
        header.v2v3.v3.maxAdverInt = htons(interval);
}

void cVrrpPacket::setChecksum (uint16_t checksum)
{
    header.chksum = htons (checksum);
}

void cVrrpPacket::addVirtualIP (const cIPv4& vip)
{
    vrIPs.push_back(vip.get());
}

void cVrrpPacket::compile (bool calcChecksum)
{
    // set mac header with vrrp ietf mac address 00:00:5e:00:01:VRID
    getFirstEthernetPacket().setDestMac (cMacAddress(0, 0, 0x5e, 0, 1, header.vrid));

    // set IPv4 Header
    setTimeToLive(255);
    setDestination(cIPv4("224.0.0.18"));

    header.countIpAddr = (uint8_t)vrIPs.size();

    // fill up authentication data (obsolete)
    if (header.getVersion() == 2)
    {
        cIPv4 nullAddr;
        addVirtualIP (nullAddr);
        addVirtualIP (nullAddr);
    }

    if (calcChecksum)
    {
        if (header.getVersion() == 2)
        {
            // calc checksum without pseudo header
            header.chksum = cInetChecksum::rfc1071((const uint8_t*)&header, sizeof(header), (const uint8_t*)vrIPs.data(), vrIPs.size() * sizeof(struct in_addr));
        }
        else
        {
            ipv4_pseudo_header_t psHeader;
            cIPv4 src, dst;
            getSource (src);
            getDestination (dst);
            psHeader.srcIp = src.get();
            psHeader.dstIp = dst.get();
            psHeader.nix   = 0;
            psHeader.protocol = PROTO_VRRP;
            psHeader.len = htons(uint16_t(sizeof(header) + vrIPs.size() * sizeof(struct in_addr)));

            // calc checksum with pseudo header
            header.chksum = cInetChecksum::rfc1071((const uint8_t*)&psHeader, sizeof(psHeader), (const uint8_t*)&header, sizeof(header), (const uint8_t*)vrIPs.data(), vrIPs.size() * sizeof(struct in_addr));
        }
    }


    cIPPacket::compile (PROTO_VRRP, (const uint8_t*)&header, sizeof(header), (const uint8_t*)vrIPs.data(), vrIPs.size() * sizeof(struct in_addr));
}


