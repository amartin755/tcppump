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


#ifndef ARP_PACKET_H_
#define ARP_PACKET_H_

#include <cstdint>

#include "ethernetpacket.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"

class cArpPacket : public cEthernetPacket
{
public:
    cArpPacket ();
    void whoHas (const cMacAddress& srcMac, const cIPv4& srcIp, const cIPv4& ip);
    void probe (const cMacAddress& srcMac, const cIPv4& ip);
    void announce (const cMacAddress& srcMac, const cIPv4& ip);
    void setAll (uint16_t opcode, const cMacAddress& srcMac, const cIPv4& srcIp, const cMacAddress& dstMac, const cIPv4& dstIp);



#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:

};

#pragma pack(1)

struct arp_t
{
    uint16_t hwType;
    uint16_t protType;
    uint8_t  hwAddrSize;
    uint8_t  protAddrSize;
    uint16_t opcode;
    cMacAddress::mac_t srcMac;
    struct in_addr srcIp;
    cMacAddress::mac_t dstMac;
    struct in_addr dstIp;

    bool isRequest (void)
    {
        return opcode == htons(1);
    }
    bool isReply (void)
    {
        return opcode == htons(2);
    }

};

#pragma pack()

#endif /* ARP_PACKET_H_ */
