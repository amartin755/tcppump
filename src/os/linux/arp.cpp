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

#include <cstring>
#include <chrono>

#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>     /* IEEE 802.3 Ethernet constants */
#include <net/if.h>

#include "arp.hpp"

#include "bug.hpp"
#include "inet.h"
#include "console.hpp"
#include "netinterface.hpp"
#include "arppacket.hpp"

using namespace std;

cArp::cArp (cNetInterface& i) : ifc(i)
{
    // We don't really need an "opened" interface here. This is a sanity check, to accept validated interfaces only.
    BUG_ON (i.isOpen ());
}

bool cArp::resolve (const cIpAddress& ip, cMacAddress& mac)
{
    bool resolved = false;
    struct sockaddr_ll device;
    int arpSock;
    cMacAddress myMac;
    cIpAddress myIP;
    BUG_ON (ifc.getMAC(myMac));
    BUG_ON (ifc.getIPv4(myIP));


    errno = 0;
    if ((arpSock = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ARP))) < 0)
    {
        Console::PrintError ("Unable to open raw socket. %s.\n", strerror(errno));
        return false;
    }

    memset (&device, 0, sizeof(device));
    device.sll_ifindex = if_nametoindex (ifc.getName());
    device.sll_family  = AF_PACKET;
    device.sll_halen   = htons (sizeof (myMac));
    memcpy (device.sll_addr, &myMac, sizeof (myMac));

    uint8_t buf[64] = {0};
    cArpPacket arpReq;
//    arp.probe (myMac, ip); // use probe instead of 'whoHas' to hide our IP address, because it could be overwritten by user
    arpReq.whoHas (myMac, myIP, ip);

    for (int n = 0; n < 2 && !resolved; n++)
    {
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        errno = 0;
        if (setsockopt (arpSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv))
        {
            Console::PrintError ("Unable to set socket timeout. %s.\n", strerror(errno));
            break;
        }

        auto tStart = chrono::steady_clock::now();
        errno = 0;
        if (sendto (arpSock, arpReq.get(), arpReq.getLength(), 0,
                (struct sockaddr *) &device, sizeof (device)) != (ssize_t)arpReq.getLength())
        {
            Console::PrintError ("error: %s\n", strerror (errno));
            break;
        }

        do
        {
            if (recv (arpSock, buf, sizeof(buf), 0) >=
                    ssize_t(sizeof(mac_header_t) + sizeof (arp_t)))
            {
                mac_header_t* mHeader = (mac_header_t*)buf;
                arp_t* arp            = (arp_t*)(buf + sizeof (mac_header_t));

                if (mHeader->ethertypeLength == htons (ETH_P_ARP) &&
                    arp->isReply()                                &&
                    cIpAddress (arp->srcIp) == ip)
                {
                    mac.set ((void*)&arp->srcMac, sizeof (arp->srcMac));
                    resolved = true;
                    break;
                }
            }
        }
        while (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - tStart).count() < 1000);
    }

    close (arpSock);

    return resolved;
}
