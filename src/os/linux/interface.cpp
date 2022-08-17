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
#include <string>
#include <cstdio>
#include <cstdlib>
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

#include "bug.hpp"
#include "console.hpp"
#include "sleep.hpp"
#include "pcapfilter.hpp"
#include "signal.hpp"


cInterface::cInterface(const char* ifname)
: name (ifname)
{
    ifcHandle   = -1;
    ifIndex     = 0;
    sentPackets = 0;
    sentBytes   = 0;
    firstPacket = true;
    mtu         = 0;
    linkSpeed   = 0;
    sendOnly    = true;
    pcapHandle  = nullptr;

    ifIndex = if_nametoindex (name.c_str ());
    if (!ifIndex)
    {
        Console::PrintError ("Unknown network interface '%s'\n", name.c_str());
    }
    else
    {
        // test access rights by opening a raw socket
        int fd;
        errno = 0;
        if ((fd = socket (PF_PACKET, SOCK_RAW, 0)) < 0)
        {
            ifIndex = 0;
            if (errno == EACCES || errno == EPERM)
                Console::PrintError ("Insufficient access rights. You need to be root or tcppump needs raw capabilities.\n");
            else
                Console::PrintError ("Unable to open raw socket. %s.\n", strerror(errno));
        }
        else
        {
            ::close (fd);
        }
    }
}

cInterface::~cInterface()
{
    close ();
}

bool cInterface::isReady (void) const
{
    return !!ifIndex;
}

bool cInterface::open (bool txOnly)
{
    // aleady open
    if (ifcHandle > 0)
        return true;

    sendOnly = txOnly;

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
    mtu       = getMTU ();
    linkSpeed = getLinkSpeed ();

    std::string macAsString;
    myMac.get(macAsString);
    Console::PrintDebug ("Successfully opened %s mac=%s\n", name.c_str(), macAsString.c_str());

    lastSentPacket.clear();

    if (!sendOnly)
    {
        char errbuf[PCAP_ERRBUF_SIZE] = {0};
        pcapHandle = pcap_create (name.c_str(), errbuf);
        if (!pcapHandle)
        {
            Console::PrintError ("Unable to open the pcap interface %s.\n", name.c_str());
            Console::PrintError ("pcap error: %s\n", errbuf);
            this->close();
        }
        if (pcap_set_timeout (pcapHandle, 1000)  ||
            pcap_set_snaplen (pcapHandle, 65536) ||
            pcap_set_immediate_mode (pcapHandle, 1))
        {
            Console::PrintError ("Error during setup of pcap interface. %s\n", pcap_geterr (pcapHandle));
        }
        int ret = pcap_activate (pcapHandle);
        if (ret)
        {
            if (ret == PCAP_ERROR_IFACE_NOT_UP)
                Console::PrintError ("Interface %s is down\n", name.c_str());
            else
                Console::PrintError ("Could not active pcap interface for capturing. %s\n", pcap_geterr (pcapHandle));
            this->close();
        }
        if (pcap_setdirection (pcapHandle, PCAP_D_IN))
        {
            Console::PrintError ("Could not set filter to receive only incoming packets. %s\n", pcap_geterr (pcapHandle));
            Console::PrintError ("We might receive our own packets.\n");
        }
    }

    return (isOpen());
}


bool cInterface::close ()
{
    // aleady closed
    if (ifcHandle > 0)
        ::close (ifcHandle);
    if (pcapHandle)
        pcap_close (pcapHandle);
    ifcHandle = -1;
    pcapHandle = nullptr;
    ifIndex = 0;
    myMac.clear ();
    myIP.clear ();

    return true;
}

bool cInterface::prepareSendQueue (__attribute__((unused)) size_t packetCnt,
        __attribute__((unused)) size_t totalBytes, __attribute__((unused)) bool synchronized)
{
    if (synchronized)
    {
        cTimeval accuracy = tcppump::SleepInit ();
        Console::PrintMostVerbose ("System timer accuracy is %u usec. For packet delays below that value we do busy waiting.\n", (unsigned)accuracy.us());
    }

    // STUB
    return true;
}

bool cInterface::sendPacket (const uint8_t* payload, size_t length, const cTimeval& t)
{
    cTimeval sleepTime (t);
    sleepTime.sub (lastSentPacket);

    if (firstPacket)
    {
        firstPacket = false;
        tStart = std::chrono::high_resolution_clock::now();
    }

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
    // update statistics
    sentPackets++;
    sentBytes += (uint64_t)length;

    Console::PrintDebug ("sent %zu bytes\n", length);

    return true;
}

bool cInterface::flushSendQueue (void)
{
    // STUB
    return true;
}


void cInterface::getSendStatistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const
{
    auto tEnd = std::chrono::high_resolution_clock::now();
    auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(tEnd - tStart);
    double seconds = (double)elapsedUs.count()/1000000.0;

    sentPackets = this->sentPackets;
    sentBytes   = this->sentBytes;
    duration    = seconds;
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
            Console::PrintDebug ("mac-error: %s\n", strerror (errno));
            return false;
        }

        errno = 0;
        memset (&ifr, 0, sizeof (ifr));
        snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", name.c_str());
        if (ioctl (s, SIOCGIFHWADDR, &ifr) < 0)
        {
            Console::PrintDebug ("mac-error: %s\n", strerror (errno));
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
            Console::PrintDebug ("ip-error: %s\n", strerror (errno));
            return false;
        }

        errno = 0;
        memset (&ifr, 0, sizeof (ifr));
        ifr.ifr_addr.sa_family = AF_INET;
        snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", name.c_str());
        if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
        {
            Console::PrintDebug ("ip-error: %s\n", strerror (errno));
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


uint32_t cInterface::getMTU (void)
{
    if (!mtu)
    {
        struct ifreq ifr;
        int s;

        errno = 0;
        if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
        {
            Console::PrintDebug ("mtu-error: %s\n", strerror (errno));
            return 0;
        }

        errno = 0;
        std::memset (&ifr, 0, sizeof (ifr));
        std::snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", name.c_str());
        if (ioctl (s, SIOCGIFMTU, &ifr) < 0)
        {
            Console::PrintDebug ("mtu-error: %s\n", strerror (errno));
            return 0;
        }

        mtu = (uint32_t)ifr.ifr_mtu;

        ::close (s);
    }
    return mtu;
}


uint64_t cInterface::getLinkSpeed (void)
{
    if (!linkSpeed)
    {
        std::string path ("/sys/class/net/");
        char s[32] = {0};

        path += name + "/speed";

        errno = 0;
        FILE* fp = std::fopen (path.c_str(), "r");

        if (!fp)
        {
            Console::PrintError ("error: %s\n", strerror (errno));
            return 0;
        }

        if (std::fread (s, 1, sizeof(s) - 1, fp))
        {
            linkSpeed = (uint64_t)std::strtoul(s, NULL, 10) * (uint64_t)1000000;
        }

        std::fclose (fp);
    }

    return linkSpeed;
}


bool cInterface::isOpen () const
{
    return (ifcHandle != -1);
}


const char* cInterface::getName (void) const
{
    return name.c_str();
}

bool cInterface::addReceiveFilter (const char* filter)
{
    if (sendOnly)
        return true;
    cPcapFilter f(pcapHandle);
    return f.compile (filter) && f.apply();
}
