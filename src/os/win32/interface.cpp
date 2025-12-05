// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2025 Andreas Martin (netnag@mailbox.org)
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


#include <string>
#include <vector>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <winsock2.h>
#include <iphlpapi.h>
#include <winerror.h>

#include "interface.hpp"

#include "bug.hpp"
#include "signal.hpp"
#include "console.hpp"
#include "ethernetpacket.hpp"
#include "libpcap.h"


class cPcapSendQueue
{
public:
    cPcapSendQueue (size_t qlen) : bytesQueued(0), packetsQueued(0), packetBytesQueued(0)
    {
        pcapQ = pcap_sendqueue_alloc ((u_int)qlen);
        if (!pcapQ)
            throw std::bad_alloc();
    }
    ~cPcapSendQueue ()
    {
        BUG_ON (!pcapQ);
        pcap_sendqueue_destroy (pcapQ);
        pcapQ = nullptr;
        bytesQueued = packetsQueued = 0;
    }
    void reset (void)
    {
        BUG_ON (!pcapQ);
        pcapQ->len = 0;
        bytesQueued = packetsQueued = 0;
    }
    void add (const struct pcap_pkthdr *pkt_header, const u_char *pkt_data)
    {
        BUG_ON (pcap_sendqueue_queue (pcapQ, pkt_header, pkt_data));  // can only fail if buffer calculation is wrong
        packetsQueued++;
        bytesQueued += pkt_header->caplen + sizeof (pcap_pkthdr);
        packetBytesQueued += pkt_header->caplen;
    }
    bool flush (pcap_t *pcapIf, int sync)
    {
        bool success = (u_int)bytesQueued == pcap_sendqueue_transmit (pcapIf, pcapQ, sync);
        reset ();
        return success;
    }
    size_t queued (void)
    {
        return bytesQueued;
    }
    size_t packets (void)
    {
        return packetsQueued;
    }
    size_t packetBytes (void)
    {
        return packetBytesQueued;
    }
private:
    size_t bytesQueued;
    size_t packetsQueued;
    size_t packetBytesQueued;
    pcap_send_queue* pcapQ;
};

class cJob
{
public:
    static const int MAX_SEND_QUEUES = 3;
    std::vector<cPcapSendQueue> allQueues;
    unsigned currSendQueueIn;
    unsigned currSendQueueOut;
    HANDLE semSpaceAvail; // at least one slot empty
    HANDLE semDataAvail;  // at least one slot full
    HANDLE sigEvent;
    HANDLE workerThread;
    HANDLE termEvent;
    pcap_t *ifcHandle;
    int synchronized;
    size_t pcapMaxQueueSize;
    bool finished;
    // statistic
    uint64_t* sentPackets;
    uint64_t* sentBytes;
    double* duration;
    LARGE_INTEGER tStart; // starttime of sending

    cJob (pcap_t *ifcHandle, size_t packetCnt, size_t totalBytes, int sync, uint64_t linkspeed,
            uint64_t* sentPackets, uint64_t* sentBytes, double* duration)
    {
        semSpaceAvail    = CreateSemaphore(NULL, MAX_SEND_QUEUES - 1, MAX_SEND_QUEUES, NULL); // -1 --> first element is ready for input
        semDataAvail     = CreateSemaphore(NULL, 0, MAX_SEND_QUEUES, NULL);
        sigEvent         = cSignal::sigintGetEventHandle ();

        finished         = false;
        currSendQueueIn  = 0;
        currSendQueueOut = 0;
        synchronized     = sync;
        pcapMaxQueueSize = size_t (linkspeed/8);
        this->ifcHandle  = ifcHandle;
        this->sentPackets= sentPackets;
        this->sentBytes  = sentBytes;
        this->duration   = duration;

        const size_t wholeNeededMemory = packetCnt > 0 ? packetCnt * sizeof (pcap_pkthdr) + totalBytes : pcapMaxQueueSize;

        // pre-allocate pcap packet queues
        if (wholeNeededMemory >= pcapMaxQueueSize)
        {
            allQueues.reserve (MAX_SEND_QUEUES);
            for (size_t n = 0; n < MAX_SEND_QUEUES; n++)
                allQueues.emplace_back (pcapMaxQueueSize);
        }
        else
        {
            allQueues.emplace_back (wholeNeededMemory);
        }
        termEvent    = CreateEvent (NULL, FALSE, FALSE, NULL);
        workerThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)cJob::thread, this, 0, NULL);
    }
    bool addPacket (const uint8_t* payload, size_t length, const cTimeval& t)
    {
        pcap_pkthdr hdr;
        hdr.caplen = (bpf_u_int32)length;
        hdr.len    = (bpf_u_int32)length;
        hdr.ts     = t.timeval ();

        cPcapSendQueue& q = allQueues.at (currSendQueueIn);
        q.add (&hdr, payload);

        if (q.queued () + cEthernetPacket::MAX_DOUBLE_TAGGED_PACKET + sizeof (pcap_pkthdr) >= pcapMaxQueueSize) // don't know the size of next packet, assume maximum length
        {
            ReleaseSemaphore (semDataAvail, 1, NULL);    // wake up worker thread

            // acquire next send queue
            HANDLE h[] = {sigEvent, semSpaceAvail};
            DWORD ret = WaitForMultipleObjects(2, h, FALSE, INFINITE);
            if (ret == WAIT_OBJECT_0 + 1)
            {
                // move in-pointer
                currSendQueueIn = (currSendQueueIn + 1) % allQueues.size ();
            }
            else
            {
                // sigint event
                finished = true;
            }
        }

        return true;
    }
    static DWORD WINAPI thread (cJob* instance)
    {
        HANDLE h[] = {instance->semDataAvail, instance->termEvent};
        uint64_t sentPackets;
        uint64_t sentBytes;
        bool first = true;

        while (1)
        {
            DWORD ret = WaitForMultipleObjects (2, h, FALSE, INFINITE);

            switch (ret)
            {
            case WAIT_OBJECT_0: // data available
                {
                    cPcapSendQueue& q = instance->allQueues.at (instance->currSendQueueOut);
                    sentPackets       = (uint64_t)q.packets();
                    sentBytes         = (uint64_t)q.packetBytes();

                    // measure start time
                    if (first)
                    {
                        first = false;
                        QueryPerformanceCounter (&instance->tStart);
                    }

                    if (q.flush (instance->ifcHandle, instance->synchronized))
                    {
                        // update statistic
                        *instance->sentBytes   += sentBytes;
                        *instance->sentPackets += sentPackets;
                    }
                    //FIXME error case

                    // move out-pointer
                    instance->currSendQueueOut = (instance->currSendQueueOut + 1) % instance->allQueues.size ();
                    // release send queue
                    ReleaseSemaphore (instance->semSpaceAvail, 1, NULL);

                    break;
                }
            case WAIT_OBJECT_0 + 1: // terminate?
                return 0;
            default:
                BUG ("???");
                return (DWORD)-1;
            }
        }
    }

    ~cJob ()
    {
        LARGE_INTEGER tEnd, freq;
        cPcapSendQueue& q = allQueues.at (currSendQueueIn);

        if (!finished && q.queued()) // is there still something to send?
        {
            // wake up worker thread
            ReleaseSemaphore (semDataAvail, 1, NULL);
        }
        SetEvent (termEvent); // tell worker thread to finish
        Console::PrintDebug ("Waiting for worker thread completion\n");

        // give worker thread time to cleanup, if SIGINT has appeared
        if (cSignal::sigintSignalled() && WaitForSingleObject (workerThread, 6000) == WAIT_TIMEOUT)
        {
            Console::PrintDebug ("Timeout! Killing worker thread\n");
            TerminateThread (workerThread, (DWORD)-2);
            WaitForSingleObject (workerThread, INFINITE); // wait for thread
        }
        else
        {
            // wait until worker thread has finished or SIGINT appeared
            HANDLE h[] = {sigEvent, workerThread};
            DWORD ret = WaitForMultipleObjects(2, h, FALSE, INFINITE);
            if (ret == WAIT_OBJECT_0)
            {
                Console::PrintError ("SIGINT, killing worker thread\n");
                TerminateThread (workerThread, (DWORD)-2);
                WaitForSingleObject (workerThread, INFINITE); // wait for thread
            } else if (ret == WAIT_OBJECT_0 + 1)
                Console::PrintDebug ("Worker thread finished\n");
        }

        // measure elapsed time
        QueryPerformanceCounter (&tEnd);
        QueryPerformanceFrequency (&freq);
        *duration = (double)(tEnd.QuadPart-tStart.QuadPart) / (double)freq.QuadPart;

        CloseHandle (workerThread);
        CloseHandle (termEvent);
        CloseHandle (semSpaceAvail);
        CloseHandle (semDataAvail);
    }

};



/*
 * note: Because WINPCAPSs interface naming is weird, I try a more user friendly approach.
 * On windows, ifname can either be the not changeable AdapterName (GUID), or the so-called FriendlyName, which is changeable by the user.
 * Both will be checked case-sensitive!
 * Examples: FriendlyName "WiFi" or "Local Area Connection 1."
 *           AdapterName "{3F4A136A-2ED5-4226-9CB2-7A511E93CD48}"
 */
cInterface::cInterface(const char* ifname, bool)
: name (ifname)
{
    ifcHandle  = nullptr;
    winNetAdapters = nullptr;
    job = nullptr;
    linkSpeed = 0;
    sentPackets = 0;
    sentBytes = 0;
    duration = 0;

    winNetAdapters = getAdapterAddresses ();
    adapterInfo    = getAdapterInfo (); // find selected adapter
    if (!adapterInfo)
    {
        Console::PrintError ("Unknown network interface '%s'\n", name.c_str());
    }
}

cInterface::~cInterface()
{
    close ();
}

bool cInterface::isReady (void) const
{
    return !!adapterInfo;
}

bool cInterface::open ()
{
    // aleady open
    if (ifcHandle)
        return true;

    if (!adapterInfo)
        return false;

    // convert windows' AdapterName to pcap known interface name
    std::string pcapIfName ("\\Device\\NPF_");
    pcapIfName += adapterInfo->AdapterName;

    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    ifcHandle = pcap_open (pcapIfName.c_str(),
            0, // if we don't want to receive any packets, thus we set the capbuf=0
            0,
            1000, NULL, errbuf);

    if (!ifcHandle)
    {
        Console::PrintError ("Unable to open the network interface. %s(%s) is not supported by WinPcap\n", name.c_str(), pcapIfName.c_str());
        Console::PrintError ("pcap error: %s\n", errbuf);
    }

    linkSpeed = (uint64_t)adapterInfo->TransmitLinkSpeed;

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
        else
        {
            sentBytes += length;
            sentPackets++;
        }

        return ret == 0;
    }
}

// packetCnt = 0 means endless loop
bool cInterface::prepareSendQueue (size_t packetCnt, size_t totalBytes, bool synchronized)
{
    BUG_ON (job); // we only support one job at a time
    BUG_ON (!ifcHandle);

    sentPackets = 0;
    sentBytes   = 0;
    duration    = 0;

    job = new cJob (ifcHandle, packetCnt, totalBytes, synchronized, linkSpeed, &sentPackets, &sentBytes, &duration);
    return !!job;
}

bool cInterface::flushSendQueue (void)
{
    delete job;
    job = nullptr;
    return true;
}

void cInterface::getSendStatistic (uint64_t& packets, uint64_t& bytes, double& dur) const
{
    packets = this->sentPackets;
    bytes   = this->sentBytes;
    dur     = this->duration;
}

bool cInterface::getMAC (cMacAddress& mac)
{
    if (!adapterInfo)
        return false;

    BUG_ON (adapterInfo->PhysicalAddressLength != cMacAddress::size());
    mac.set ((void*)adapterInfo->PhysicalAddress, adapterInfo->PhysicalAddressLength);

    return true;
}

bool cInterface::getIPv4 (cIPv4& ip)
{
    if (!adapterInfo)
        return false;

    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = adapterInfo->FirstUnicastAddress;
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

bool cInterface::getIPv6 (cIPv6& ip)
{
    if (!adapterInfo)
        return false;

    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = adapterInfo->FirstUnicastAddress;
    while (pUnicast)
    {
         if (pUnicast->Address.lpSockaddr && pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
         {
            struct sockaddr_in6* addr = (struct sockaddr_in6*)pUnicast->Address.lpSockaddr;
            ip.set (addr->sin6_addr);
            return true;
         }
         pUnicast = pUnicast->Next;
    }
    return false;
}

uint32_t cInterface::getMTU (void)
{
    if (!adapterInfo)
        return 0;

    return (uint32_t)adapterInfo->Mtu;
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

const char* cInterface::getName (void) const
{
    return name.c_str();
}
