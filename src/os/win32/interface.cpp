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
#include <string>
#include <pcap.h>
#include <iphlpapi.h>
#include <winerror.h>

#include "interface.hpp"
#include "console.hpp"


cInterface::cInterface(const char* ifname)
: name (ifname)
{
	ifcHandle  = NULL;
	winNetAdapters = NULL;
}

cInterface::~cInterface()
{
	close ();
}

bool cInterface::open ()
{
	// aleady open
	if (ifcHandle)
		return true;

	char errbuf[PCAP_ERRBUF_SIZE] = {0};
	ifcHandle = pcap_open_live(name.c_str(), 65536,	1, 1000, errbuf);

	if (!ifcHandle)
	{
		nn::Console::PrintError ("Unable to open the adapter. %s is not supported by WinPcap\n", name.c_str());
		nn::Console::PrintError ("pcap error: %s\n", errbuf);
	}
	else
	{
		bool error = true;
		ULONG ret;
		ULONG size = 0;

		ret = GetAdaptersAddresses (AF_UNSPEC, 0, NULL, NULL, &size);
		if (!size)
		{
			nn::Console::PrintError ("Could not determine size of adapter data\n");
		}
		else
		{
			winNetAdapters = (IP_ADAPTER_ADDRESSES *)malloc (size);
			if (!winNetAdapters)
			{
				nn::Console::PrintError ("Not enough memory\n");
			}
			else
			{
				memset (winNetAdapters, 0, size);

				ret = GetAdaptersAddresses (AF_UNSPEC, 0, NULL, winNetAdapters, &size);
				if (ret != NO_ERROR)
				{
					nn::Console::PrintError ("Call to GetAdaptersAddresses failed with error\n");
				}
				else
				{
					error = false;
				}
			}
		}
		if (error)
		{
			close ();
			ifcHandle = NULL;
		}
	}

	return (ifcHandle != NULL);
}

bool cInterface::close ()
{
	// aleady closed
	if (!ifcHandle)
		return true;

	pcap_close(ifcHandle);
	ifcHandle = NULL;

	// free windows network adapter list
	if (winNetAdapters)
		free (winNetAdapters);
	winNetAdapters = NULL;

	return true;
}

bool cInterface::sendPacket (const uint8_t* payload, size_t length)
{
	int ret = pcap_sendpacket (ifcHandle, (u_char*)payload, length);

	if (ret == -1)
		nn::Console::PrintError ("pcap error: %s\n", pcap_geterr (ifcHandle));

	return ret == 0;
}

bool cInterface::getMAC (mac_t *mac)
{
	PIP_ADAPTER_ADDRESSES pAdapterInfo = getAdapterInfo ();
	if (!pAdapterInfo)
		return false;

	assert (pAdapterInfo->PhysicalAddressLength == sizeof (mac_t));
	memcpy (mac, pAdapterInfo->PhysicalAddress, sizeof (mac_t));

	return true;
}


PIP_ADAPTER_ADDRESSES cInterface::getAdapterInfo ()
{
	PIP_ADAPTER_ADDRESSES pCurr = winNetAdapters;

	if (ifcHandle)
	{
		while (pCurr)
		{
			if (strstr (name.c_str(), pCurr->AdapterName))
				return pCurr;

			pCurr = pCurr->Next;
		}
	}
	return NULL;
}
