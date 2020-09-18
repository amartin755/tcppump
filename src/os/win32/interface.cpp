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


#include <string>
#include <pcap.h>
#include <iphlpapi.h>
#include <winerror.h>

#include "bugon.h"
#include "interface.hpp"
#include "console.hpp"

/*
 * note: Because WINPCAPSs interface naming is weired, I try a more user friendly approach.
 * On windows, ifname can either be the not changeable AdapterName (GUID), or the so-called FriendlyName, which is changeable by the user.
 * Both will be checked case-sensitive!
 * Examples: FriendlyName "WiFi" or "Local Area Connection 1."
 *           AdapterName "{3F4A136A-2ED5-4226-9CB2-7A511E93CD48}"
 */
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

    winNetAdapters = getAdapterAddresses ();
    if (!winNetAdapters)
        return false;

    // find selected adapter
    PIP_ADAPTER_ADDRESSES adapter = getAdapterInfo ();
    if (!adapter)
    {
        Console::PrintError ("Unknown network interface '%s'\n", name.c_str());
        return false;
    }

    // convert windows' AdapterName to pcap known interface name
    std::string pcapIfName("\\Device\\NPF_");
    pcapIfName += adapter->AdapterName;

    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    ifcHandle = pcap_open_live(pcapIfName.c_str(), 65536,    1, 1000, errbuf);

    if (!ifcHandle)
    {
        Console::PrintError ("Unable to open the network interface. %s(%s) is not supported by WinPcap\n", name.c_str(), pcapIfName.c_str());
        Console::PrintError ("pcap error: %s\n", errbuf);
    }

    return ifcHandle != NULL;
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

bool cInterface::sendPacket (const uint8_t* payload, size_t length) const
{
    int ret = pcap_sendpacket (ifcHandle, (u_char*)payload, (int)length);

    if (ret == -1)
        Console::PrintError ("pcap error: %s\n", pcap_geterr (ifcHandle));

    return ret == 0;
}

bool cInterface::getMAC (cMacAddress& mac)
{
    PIP_ADAPTER_ADDRESSES pAdapterInfo = getAdapterInfo ();
    if (!pAdapterInfo)
        return false;

    BUG_ON (pAdapterInfo->PhysicalAddressLength == cMacAddress::size());
    mac.set ((void*)pAdapterInfo->PhysicalAddress, pAdapterInfo->PhysicalAddressLength);

    return true;
}

bool cInterface::getIPv4 (cIpAddress& ip)
{
    PIP_ADAPTER_ADDRESSES pAdapterInfo = getAdapterInfo ();
    if (!pAdapterInfo)
        return false;

    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapterInfo->FirstUnicastAddress;
    while (pUnicast)
    {
         if (pUnicast->Address.lpSockaddr && pUnicast->Address.lpSockaddr->sa_family == AF_INET)
         {
             struct sockaddr_in* addr = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
             // filter Zeroconf addresses
             if ((ntohl ((u_long)addr->sin_addr.s_addr) & 0xFFFF0000) != 0xA9FE0000)
             {
                 ip.set (addr->sin_addr);
                 return true;
             }
         }
         pUnicast = pUnicast->Next;
    }
    return false;
}

PIP_ADAPTER_ADDRESSES cInterface::getAdapterAddresses ()
{
    ULONG ret;
    ULONG size = 0;
    PIP_ADAPTER_ADDRESSES addresses = NULL;

    ret = GetAdaptersAddresses (AF_UNSPEC, 0, NULL, NULL, &size);
    if (!size)
    {
        Console::PrintError ("Could not determine size of adapter data\n");
    }
    else
    {
        addresses = (IP_ADAPTER_ADDRESSES *)malloc (size);
        if (!addresses)
        {
            Console::PrintError ("Not enough memory\n");
        }
        else
        {
            memset (addresses, 0, size);

            ret = GetAdaptersAddresses (AF_UNSPEC, 0, NULL, addresses, &size);
            if (ret != NO_ERROR)
            {
                Console::PrintError ("Call to GetAdaptersAddresses failed with error\n");
                free (addresses);
                addresses = NULL;
            }
        }
    }
    return addresses;
}

PIP_ADAPTER_ADDRESSES cInterface::getAdapterInfo ()
{
    PIP_ADAPTER_ADDRESSES pCurr = winNetAdapters;

    while (pCurr)
    {
        if (!name.compare (pCurr->AdapterName))
            return pCurr;
        else
        {
            // FriendlyName is a wide string. Therefore we have to convert our interface name
            std::wstring wname(name.begin(), name.end());
            if (!wname.compare (pCurr->FriendlyName))
                return pCurr;
        }

        pCurr = pCurr->Next;
    }
    return NULL;
}
