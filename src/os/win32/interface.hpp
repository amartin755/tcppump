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
#include <thread>
#include <cstdint>
#include <cstddef>
#include <iphlpapi.h>

#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "timeval.hpp"

// forward declarations
typedef struct pcap pcap_t;
struct pcap_send_queue;
class cJob;

class cInterface
{

public:
    cInterface(const char* ifname);
    virtual ~cInterface();
    bool open ();
    bool close ();
    bool sendPacket (const uint8_t* payload, size_t length, const cTimeval& t);
    bool prepareSendQueue (size_t packetCnt, size_t totalBytes, bool synchronized);
    bool flushSendQueue (void);
    bool getMAC (cMacAddress&);
    bool getIPv4 (cIpAddress&);
    bool isOpen () const;

    void operator()();


private:
    PIP_ADAPTER_ADDRESSES getAdapterInfo ();
    PIP_ADAPTER_ADDRESSES getAdapterAddresses ();
    uint64_t getLinkSpeed (const char* adapterName);

    std::string name;
    pcap_t *ifcHandle;
    PIP_ADAPTER_ADDRESSES winNetAdapters;
    uint64_t linkSpeed;

    cJob* job;
};

#endif /* INTERFACE_H_ */
