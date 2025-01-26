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


#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <string>
#include <list>
#include <cstdint>
#include <cstddef>
#include <winsock2.h>
#include <iphlpapi.h>

#include "netinterface.hpp"

// forward declarations
typedef struct pcap pcap_t;
struct pcap_send_queue;
class cJob;

class cInterface : public cNetInterface
{

public:
    cInterface(const char* ifname, bool needPriviledges);
    virtual ~cInterface();
    bool open ();
    bool close ();
    bool sendPacket (const uint8_t* payload, size_t length, const cTimeval& t);
    bool prepareSendQueue (size_t packetCnt, size_t totalBytes, bool synchronized);
    bool flushSendQueue (void);
    void getSendStatistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const;
    bool getMAC (cMacAddress&);
    bool getIPv4 (cIPv4&);
    bool getIPv6 (cIPv6&);
    uint32_t getMTU (void);
    bool isOpen () const;
    const char* getName (void) const;
    bool isReady (void) const;

private:
    PIP_ADAPTER_ADDRESSES getAdapterInfo ();
    PIP_ADAPTER_ADDRESSES getAdapterAddresses ();
    uint64_t getLinkSpeed (const char* adapterName);

    std::string name;
    pcap_t *ifcHandle;
    PIP_ADAPTER_ADDRESSES winNetAdapters;
    PIP_ADAPTER_ADDRESSES adapterInfo;
    uint64_t linkSpeed;

    uint64_t sentPackets;
    uint64_t sentBytes;
    double   duration;

    cJob* job;
};

#endif /* INTERFACE_H_ */
