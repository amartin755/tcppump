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
#include <string>
#include <errno.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>           // struct ifreq
#include <netinet/in.h>
#include <net/if_arp.h>       // ARPHRD_ETHER

#include "interface.hpp"

#include "bug.h"
#include "console.hpp"
#include "sleep.hpp"


cInterface::cInterface(const char* ifname)
: name (ifname)
{
    ifcHandle  = -1;
    ifIndex    = 0;
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
        Console::PrintError ("Unknown interface %s\n", name.c_str());
        close ();
        return false;
    }
    // open raw socket for tx only
    // thus we set proto = 0 and size of receive buffer to zero
    errno = 0;
    if ((ifcHandle = socket (PF_PACKET, SOCK_RAW, 0)) < 0)
    {
        Console::PrintError ("Unable to open raw socket. %s.\n", strerror(errno));
        close ();
        return false;
    }
    int opt = 0;
    setsockopt (ifcHandle, SOL_SOCKET, SO_RCVBUF, &opt, sizeof (opt));

    getMAC (myMac);
    getIPv4 (myIP);

    std::string macAsString;
    myMac.get(macAsString);
    Console::PrintDebug ("Successfully openend %s mac=%s\n", name.c_str(), macAsString.c_str());

    lastSentPacket.clear();

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
    myMac.clear ();
    myIP.clear ();

    return true;
}

bool cInterface::sendPacket (const uint8_t* payload, size_t length, const cTimeval& t)
{
    cTimeval sleepTime (t);
    sleepTime.sub (lastSentPacket);

    if (!sleepTime.isNull())
    {
        tcppump::Sleep (sleepTime);
        lastSentPacket.add(sleepTime);
    }

    struct sockaddr_ll device;

    memset (&device, 0, sizeof(device));

    device.sll_ifindex = ifIndex;
    device.sll_family  = AF_PACKET;
    device.sll_halen   = htons (sizeof (myMac));
    memcpy (device.sll_addr, &myMac, sizeof (myMac));

    errno = 0;
    if (sendto (ifcHandle, payload, length, 0, (struct sockaddr *) &device, sizeof (device)) != (ssize_t)length)
    {
        Console::PrintError ("error: %s\n", strerror (errno));
        return false;
    }

    Console::PrintDebug ("sent %zu bytes\n", length);

    return true;
}

bool cInterface::getMAC (cMacAddress &mac)
{
    if (myMac.isNull ())
    {
        struct ifreq ifr;
        int s;

        errno = 0;
        if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
        {
            Console::PrintError ("error: %s\n", strerror (errno));
            return false;
        }

        // Use ioctl() to look up interface name and get its MAC address.
        errno = 0;
        memset (&ifr, 0, sizeof (ifr));
        snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", name.c_str());
        if (ioctl (s, SIOCGIFHWADDR, &ifr) < 0)
        {
            Console::PrintError ("error: %s\n", strerror (errno));
            return false;
           }

        ::close (s);

        if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER && ifr.ifr_hwaddr.sa_family != ARPHRD_LOOPBACK)
            return false;

        // Copy source MAC address
        mac.set ((void*)ifr.ifr_hwaddr.sa_data, mac.size());
    }
    else
    {
        mac.set (myMac);
    }
    return true;
}

bool cInterface::getIPv4 (cIpAddress &ip)
{
    if (myIP.isNull())
    {
        struct ifreq ifr;
        int s;

        errno = 0;
        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            Console::PrintError ("error: %s\n", strerror (errno));
            return false;
        }

        // Use ioctl() to look up interface name and get its IPv4 address.
        errno = 0;
        memset (&ifr, 0, sizeof (ifr));
        ifr.ifr_addr.sa_family = AF_INET;
        snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", name.c_str());
        if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
        {
            Console::PrintError ("error: %s\n", strerror (errno));
            return false;
           }

        ip.set(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr);

        ::close (s);
    }
    else
    {
        ip.set(myIP);
    }
    return true;
}


bool cInterface::isOpen () const
{
    return (ifcHandle != -1);
}
