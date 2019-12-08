/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2019 Andreas Martin (netnag@mailbox.org)
 *
 * interface.cpp
 *
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

#include <cassert>
#include <cstring>
#include <string>
#include <errno.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>           // struct ifreq
#include <netinet/in.h>

#include "interface.hpp"
#include "console.hpp"


cInterface::cInterface(const char* ifname)
: name (ifname)
{
    ifcHandle  = -1;
    ifIndex    = 0;
    myMac.set (0);
    myIP = 0;
}

cInterface::~cInterface()
{
    close ();
}

bool cInterface::open ()
{
    // aleady open
    if (ifcHandle != -1)
        return true;

    ifIndex = if_nametoindex (name.c_str ());
    if (!ifIndex)
    {
        nn::Console::PrintError ("Unknown interface %s\n", name.c_str());
        close ();
    }
    errno = 0;
    if ((ifcHandle = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0)
    {
        nn::Console::PrintError ("Unable to open raw socket. %s.\n", strerror(errno));
        ifIndex = 0;
    }
    getMAC (&myMac);
    getIPv4 (&myIP);

    nn::Console::PrintDebug ("Successfully openend %s mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
        name.c_str(), myMac.a, myMac.b, myMac.c, myMac.d, myMac.e, myMac.f);

    return (ifcHandle != -1);
}

bool cInterface::close ()
{
    // aleady closed
    if (!ifcHandle)
        return true;

    ::close (ifcHandle);
    ifcHandle = -1;
    ifIndex = 0;
    myMac.set (0);

    return true;
}

bool cInterface::sendPacket (const uint8_t* payload, size_t length) const
{
    struct sockaddr_ll device;

    memset (&device, 0, sizeof(device));

    device.sll_ifindex = ifIndex;
    device.sll_family  = AF_PACKET;
    device.sll_halen   = htons (sizeof (myMac));
    memcpy (device.sll_addr, &myMac, sizeof (myMac));

    errno = 0;
    if (sendto (ifcHandle, payload, length, 0, (struct sockaddr *) &device, sizeof (device)) != (ssize_t)length)
    {
        nn::Console::PrintError ("error: %s\n", strerror (errno));
        return false;
    }

    nn::Console::PrintDebug ("sent %zu bytes\n", length);

    return true;
}

bool cInterface::getMAC (mac_t *mac)
{
    if (myMac.isNull ())
    {
        struct ifreq ifr;
        int s;

        errno = 0;
        if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
        {
            nn::Console::PrintError ("error: %s\n", strerror (errno));
            return false;
        }

        // Use ioctl() to look up interface name and get its MAC address.
        errno = 0;
        memset (&ifr, 0, sizeof (ifr));
        snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", name.c_str());
        if (ioctl (s, SIOCGIFHWADDR, &ifr) < 0)
        {
            nn::Console::PrintError ("error: %s\n", strerror (errno));
            return false;
           }

        // Copy source MAC address.
        memcpy (mac, ifr.ifr_hwaddr.sa_data, sizeof (*mac));

        ::close (s);
    }
    else
    {
        *mac = myMac;
    }
    return true;
}

bool cInterface::getIPv4 (ipv4_t *ip)
{
    if (!myIP)
    {
        struct ifreq ifr;
        int s;

        errno = 0;
        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            nn::Console::PrintError ("error: %s\n", strerror (errno));
            return false;
        }

        // Use ioctl() to look up interface name and get its IPv4 address.
        errno = 0;
        memset (&ifr, 0, sizeof (ifr));
        ifr.ifr_addr.sa_family = AF_INET;
        snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", name.c_str());
        if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
        {
            nn::Console::PrintError ("error: %s\n", strerror (errno));
            return false;
           }

        *ip = (ipv4_t)((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr;

        ::close (s);
    }
    else
    {
        *ip = myIP;
    }
    return true;
}
