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


#ifndef NETINTERFACE_H_
#define NETINTERFACE_H_

#include <string>
#include <list>
#include <cstdint>
#include <cstddef>

#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "timeval.hpp"
#include "pcapfilter.hpp"

// forward declarations
typedef struct pcap pcap_t;

class cNetInterface
{

public:
    static cNetInterface* create(const char* ifname, bool needPriviledges);
    virtual ~cNetInterface () {};
    virtual bool open (bool sendOnly) = 0;
    virtual bool close () = 0;
    virtual bool sendPacket (const uint8_t* payload, size_t length, const cTimeval& t) = 0;
    virtual bool prepareSendQueue (size_t packetCnt, size_t totalBytes, bool synchronized) = 0;
    virtual bool flushSendQueue (void) = 0;
    virtual void getSendStatistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const = 0;
    virtual bool getMAC (cMacAddress&) = 0;
    virtual bool getIPv4 (cIPv4&) = 0;
    virtual uint32_t getMTU (void) = 0;
    virtual bool isOpen () const = 0;
    virtual bool addReceiveFilter (const char* filter) = 0;
    virtual const char* getName (void) const = 0;
    virtual bool isReady (void) const = 0;
    bool waitForPacket (void);
    const uint8_t* receivePacket (cTimeval* timestamp, int* len, const cPcapFilter* filter = nullptr, const cTimeval* dropBefore = nullptr);

private:
    virtual pcap_t* getCaptureInterface (void) = 0;

};

#endif /* NETINTERFACE_H_ */
