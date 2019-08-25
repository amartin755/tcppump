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

	if ((ifcHandle = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) 
	{
		nn::Console::PrintError ("Unable to open the interface.\n", name.c_str());
	}

	ifIndex = if_nametoindex (name.c_str ());
	if (!ifIndex)
	{
		nn::Console::PrintError ("Could not determine interface index\n");
		close ();
		ifcHandle = -1;
	}

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

	return true;
}

bool cInterface::sendPacket (const uint8_t* payload, size_t length)
{
	struct sockaddr_ll device;

	device.sll_ifindex = ifIndex;

	int ret = 0;//pcap_sendpacket (ifcHandle, (u_char*)payload, length);

//	if (ret == -1)
//		nn::Console::PrintError ("pcap error: %s\n", pcap_geterr (ifcHandle));

	return ret == 0;
}

bool cInterface::getMAC (mac_t *mac)
{
	struct ifreq ifr;
	int s;

	if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) 
	{
		return false;
	}

	// Use ioctl() to look up interface name and get its MAC address.
	memset (&ifr, 0, sizeof (ifr));
	snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", name.c_str());
	if (ioctl (s, SIOCGIFHWADDR, &ifr) < 0)
	{
		return false;
   	}

	// Copy source MAC address.
	memcpy (mac, ifr.ifr_hwaddr.sa_data, sizeof (*mac));

	::close (s);

	return true;
}
