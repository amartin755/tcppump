/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2020 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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
    void whoHas (const cMacAddress& srcMac, const cIpAddress& srcIp, const cIpAddress& ip);
    void probe (const cMacAddress& srcMac, const cIpAddress& ip);
    void announce (const cMacAddress& srcMac, const cIpAddress& ip);
    void setAll (uint16_t opcode, const cMacAddress& srcMac, const cIpAddress& srcIp, const cMacAddress& dstMac, const cIpAddress& dstIp);



#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:

};

#pragma pack(1)

typedef struct
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

}arp_t;

#pragma pack()

#endif /* ARP_PACKET_H_ */
