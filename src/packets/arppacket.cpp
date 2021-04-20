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

#include "arppacket.hpp"
#include "bug.hpp"



cArpPacket::cArpPacket ()
{

}


void cArpPacket::whoHas (const cMacAddress& srcMac, const cIpAddress& srcIp, const cIpAddress& ip)
{
    cMacAddress dstMac;
    setAll (1, srcMac, srcIp, dstMac, ip);
}

void cArpPacket::probe (const cMacAddress& srcMac, const cIpAddress& ip)
{
    cMacAddress dstMac;
    setAll (1, srcMac, cIpAddress(), dstMac, ip);
}


void cArpPacket::announce (const cMacAddress& srcMac, const cIpAddress& ip)
{
    cMacAddress dstMac;
    setAll (1, srcMac, ip, dstMac, ip);
}


void cArpPacket::setAll (uint16_t opcode, const cMacAddress& srcMac, const cIpAddress& srcIp, const cMacAddress& dstMac, const cIpAddress& dstIp)
{
    arp_t a;
    a.hwType       = htons (1);
    a.protType     = htons (ETHERTYPE_IPV4);
    a.hwAddrSize   = (uint8_t)cMacAddress::size();
    a.protAddrSize = (uint8_t)sizeof (a.dstIp);
    a.opcode       = htons (opcode);
    a.srcIp        = srcIp.get ();
    a.dstIp        = dstIp.get ();
    srcMac.get(&a.srcMac);
    dstMac.get(&a.dstMac);

    this->setMacHeader (srcMac, dstMac.isNull() ? cMacAddress(0xffu) : dstMac);
    this->setTypeLength (ETHERTYPE_ARP);
    this->setPayload ((uint8_t*)&a, sizeof(a));
}


#ifdef WITH_UNITTESTS
#include "console.hpp"
void cArpPacket::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

}
#endif
