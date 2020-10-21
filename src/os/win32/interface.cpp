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
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <pcap.h>
#include <Win32-Extensions.h>
#include <iphlpapi.h>
#include <winerror.h>

#include "interface.hpp"

#include "bug.h"
#include "console.hpp"
#include "ethernetpacket.hpp"



class cJob
{
    cQueue<pcap_send_queue*, 3> *pMsgQ;
    pcap_send_queue *currSendQueue;
    size_t bytesInCurrSendQueue;
    std::thread *workerThread;
    pcap_t *ifcHandle;
    size_t bytesQueued;
    size_t bytesSent;
    int synchronized;
    unsigned pcapMaxQueueSize;

public:
    cJob (pcap_t *ifcHandle, int packetCnt, size_t totalBytes, int sync, uint64_t linkspeed)
    {
        pcapMaxQueueSize = linkspeed/8;
        this->ifcHandle = ifcHandle;
        pMsgQ = new cQueue<pcap_send_queue*, 3>();

        const size_t wholeNeededMemory = packetCnt > 0 ? packetCnt * sizeof (pcap_pkthdr) + totalBytes : pcapMaxQueueSize;

        currSendQueue = pcap_sendqueue_alloc (wholeNeededMemory >= pcapMaxQueueSize ? pcapMaxQueueSize : wholeNeededMemory);
        if (!currSendQueue)
            throw std::bad_alloc();

        bytesInCurrSendQueue = 0;
        bytesSent            = 0;
        bytesQueued          = 0;
        synchronized         = sync;

        workerThread = new std::thread (std::ref(*this));
    }
    bool addPacket (const uint8_t* payload, size_t length, const cTimeval& t)
    {
        if (!currSendQueue)
        {
            currSendQueue = pcap_sendqueue_alloc (pcapMaxQueueSize);
            bytesInCurrSendQueue = 0;
        }
        pcap_pkthdr hdr;
        hdr.caplen = length;
        hdr.len    = length;
        hdr.ts     = t.timeval ();

        BUG_ON (!pcap_sendqueue_queue (currSendQueue, &hdr, payload)); // can only fail if buffer calculation is wrong
        bytesInCurrSendQueue += length + sizeof (pcap_pkthdr);
        bytesQueued          += length + sizeof (pcap_pkthdr);

        if (bytesInCurrSendQueue + cEthernetPacket::MAX_DOUBLE_TAGGED_PACKET + sizeof (pcap_pkthdr) >= pcapMaxQueueSize) // don't know the size of next packet, assume maximum length
        {
            pMsgQ->push (currSendQueue);
            currSendQueue = nullptr;
        }

        return true;
    }
    void operator ()()
    {
        pcap_send_queue *sendQueue;
        while ((sendQueue = pMsgQ->pop ()))
        {
            bytesSent += pcap_sendqueue_transmit (ifcHandle, sendQueue, synchronized);
            pcap_sendqueue_destroy (sendQueue);
        }
    }

    ~cJob ()
    {
        if (currSendQueue) // is there something to send?
        {
            pMsgQ->push (currSendQueue);
            currSendQueue = nullptr;
        }
        pMsgQ->push (nullptr); // send thread term signal
        workerThread->join ();

        delete pMsgQ;
        delete workerThread;
    }

};



/*
 * note: Because WINPCAPSs interface naming is weird, I try a more user friendly approach.
 * On windows, ifname can either be the not changeable AdapterName (GUID), or the so-called FriendlyName, which is changeable by the user.
 * Both will be checked case-sensitive!
 * Examples: FriendlyName "WiFi" or "Local Area Connection 1."
 *           AdapterName "{3F4A136A-2ED5-4226-9CB2-7A511E93CD48}"
 */
cInterface::cInterface(const char* ifname)
: name (ifname)
{
    ifcHandle  = nullptr;
    winNetAdapters = nullptr;
    job = nullptr;
    linkSpeed = 0;
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
    // we don't want to receive any packets, thus we set the capbuf=0
    ifcHandle = pcap_open_live(pcapIfName.c_str(), 0, 0, 1000, errbuf);

    if (!ifcHandle)
    {
        Console::PrintError ("Unable to open the network interface. %s(%s) is not supported by WinPcap\n", name.c_str(), pcapIfName.c_str());
        Console::PrintError ("pcap error: %s\n", errbuf);
    }

    linkSpeed = getLinkSpeed (adapter->AdapterName);

    return ifcHandle != NULL;
}

bool cInterface::close ()
{
    // aleady closed
    if (!ifcHandle)
        return true;

    flushSendQueue();

    pcap_close(ifcHandle);
    ifcHandle = nullptr;

    // free windows network adapter list
    if (winNetAdapters)
        free (winNetAdapters);
    winNetAdapters = nullptr;


    return true;
}

bool cInterface::sendPacket (const uint8_t* payload, size_t length, const cTimeval& t)
{
    if (job) // queued send?
    {
        return job->addPacket (payload, length, t);
    }
    else
    {
        int ret = pcap_sendpacket (ifcHandle, (u_char*)payload, (int)length);

        if (ret == -1)
            Console::PrintError ("pcap error: %s\n", pcap_geterr (ifcHandle));

        return ret == 0;
    }
}

// packetCnt = 0 means endless loop
bool cInterface::prepareSendQueue (int packetCnt, size_t totalBytes, bool synchronized)
{
    BUG_ON (!job); // we only support one job at a time
    BUG_ON (ifcHandle);

    job = new cJob (ifcHandle, packetCnt, totalBytes, synchronized, linkSpeed);
    return !!job;
}

bool cInterface::flushSendQueue (void)
{
    delete job;
    job = nullptr;
    return true;
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

bool cInterface::isOpen () const
{
    return ifcHandle != NULL;
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

uint64_t cInterface::getLinkSpeed (const char* guid)
{
    FILE *fp;
    char speed[16] = {0};

    // wmic NIC where GUID="{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}" get Speed | find /v "Speed"
    std::string cmd = "wmic NIC where GUID=\"";
    cmd.append (guid);
    cmd.append ("\" get Speed");

    fp = _popen(cmd.c_str (), "rt");
    if (fp == NULL)
        return 0;

    int c;

    while ((c = std::getc(fp)) != EOF)
    {
        if (std::isdigit(c))
        {
            unsigned cnt = 0;
            do
            {
                speed[cnt++] = c;
                if (cnt >= (sizeof (speed) - 1))
                    break;
            }
            while (std::isdigit(c = std::getc(fp)));
            break;
        }
    }
    _pclose(fp);

    return (uint64_t)std::strtoull(speed, NULL, 10);
}
