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
#include <cstdint>
#include <cstddef>
#include <chrono>

#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "timeval.hpp"
#include "netinterface.hpp"



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
    uint64_t getLinkSpeed (void);
    bool isOpen () const;
    const char* getName (void) const;
    bool isReady (void) const;

private:
    std::string name;
    int ifcHandle;
    int ifIndex;
    cMacAddress myMac;
    cIPv4 myIP;
    cIPv6 myIPv6;
    uint32_t mtu;
    uint64_t linkSpeed;
    cTimeval lastSentPacket;

    bool firstPacket;
    std::chrono::high_resolution_clock::time_point tStart;
    uint64_t sentPackets;
    uint64_t sentBytes;
};

#endif /* INTERFACE_H_ */

