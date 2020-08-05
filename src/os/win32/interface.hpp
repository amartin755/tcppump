/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * interface.hpp
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

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <string>
#include <cstdint>
#include <cstddef>
#include <iphlpapi.h>

#include "ipaddress.hpp"
#include "macaddress.hpp"

typedef struct pcap pcap_t;
//typedef struct _IP_ADAPTER_ADDRESSES *PIP_ADAPTER_ADDRESSES;

class cInterface
{
public:
    cInterface(const char* ifname);
    virtual ~cInterface();
    bool open ();
    bool close ();
    bool sendPacket (const uint8_t* payload, size_t length);
    bool getMAC (cMacAddress&);
    bool getIPv4 (cIpAddress&);

private:
    PIP_ADAPTER_ADDRESSES getAdapterInfo ();
    PIP_ADAPTER_ADDRESSES getAdapterAddresses ();

    std::string name;
    pcap_t *ifcHandle;
    PIP_ADAPTER_ADDRESSES winNetAdapters;

};

#endif /* INTERFACE_H_ */
