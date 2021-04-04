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


// forward declarations
typedef struct pcap pcap_t;


class cInterface : public cNetInterface
{
public:
    cInterface(const char* ifname);
    virtual ~cInterface();
    bool open (bool sendOnly);
    bool close ();
    bool sendPacket (const uint8_t* payload, size_t length, const cTimeval& t);
    bool prepareSendQueue (size_t packetCnt, size_t totalBytes, bool synchronized);
    bool flushSendQueue (void);
    void getSendStatistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const;
    bool getMAC (cMacAddress&);
    bool getIPv4 (cIpAddress&);
    uint32_t getMTU (void);
    uint64_t getLinkSpeed (void);
    bool isOpen () const;
    const char* getName (void) const;
    bool waitForPacket (void);
    const uint8_t* receivePacket (cTimeval* timestamp, int* len, const cPcapFilter* filter = nullptr);
    bool addReceiveFilter (const char* filter);
    bool addReceiveFilter (bool tcp, bool udp,
                           const std::list<const char*>* ethertypes,
                           const std::list<const char*>* hostsMAC,
                           const std::list<const char*>* hostsIP);

private:

    std::string name;
    int ifcHandle;
    pcap_t *pcapHandle;
    int ifIndex;
    cMacAddress myMac;
    cIpAddress myIP;
    uint32_t mtu;
    uint64_t linkSpeed;
    cTimeval lastSentPacket;

    bool firstPacket;
    std::chrono::high_resolution_clock::time_point tStart;
    uint64_t sentPackets;
    uint64_t sentBytes;
    bool sendOnly;
};

#endif /* INTERFACE_H_ */

