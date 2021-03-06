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


#include <string>
#include <vector>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#define HAVE_REMOTE 1
#include <pcap.h>

#include "bug.hpp"
#include "console.hpp"
#include "signal.hpp"
#include "netinterface.hpp"
#include "interface.hpp"

cNetInterface* cNetInterface::create(const char* ifname)
{
    return new cInterface (ifname);
}

const uint8_t* cNetInterface::receivePacket (cTimeval* timestamp, int* len, const cPcapFilter* filter, const cTimeval* dropBefore)
{
    struct pcap_pkthdr *header;
    const u_char *pkt_data;
    int res;
    pcap_t *ifcHandle = this->getCaptureInterface();


    do
    {
        if (cSignal::sigintSignalled ())
            return nullptr;

        res = pcap_next_ex(ifcHandle, &header, &pkt_data);
        if (res > 0)
            if ( (dropBefore && (cTimeval(header->ts) < *dropBefore)) ||
                     (filter && !filter->match(header, pkt_data)) )
                res = 0;
    }
    while (!res);


    if (res < 0)
    {
        Console::PrintError ("Error while reading packets: %s\n", pcap_geterr (ifcHandle));
        return nullptr;
    }

    if (timestamp)
        timestamp->set (header->ts);
    if (len)
        *len = (int)header->len;

    return (uint8_t*)pkt_data;
}

bool cNetInterface::waitForPacket (void)
{
    return !!receivePacket (nullptr, nullptr);
}
