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


#ifndef NETINTERFACE_H_
#define NETINTERFACE_H_

#include <string>
#include <list>
#include <cstdint>
#include <cstddef>

#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "timeval.hpp"


class cNetInterface
{

public:
    static cNetInterface* factory(const char* ifname, bool sendOnly);
    virtual ~cNetInterface () {};
    virtual bool open () = 0;
    virtual bool close () = 0;
    virtual bool sendPacket (const uint8_t* payload, size_t length, const cTimeval& t) = 0;
    virtual bool prepareSendQueue (size_t packetCnt, size_t totalBytes, bool synchronized) = 0;
    virtual bool flushSendQueue (void) = 0;
    virtual void getSendStatistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const = 0;
    virtual bool getMAC (cMacAddress&) = 0;
    virtual bool getIPv4 (cIpAddress&) = 0;
    virtual uint32_t getMTU (void) = 0;
    virtual bool isOpen () const = 0;
    virtual bool waitForPacket (void) = 0;
    virtual const uint8_t* receivePacket (cTimeval* timestamp, int* len) = 0;
    virtual bool addReceiveFilter (const char* filter) = 0;
    virtual bool addReceiveFilter (bool tcp, bool udp,
                           const std::list<const char*>* ethertypes,
                           const std::list<const char*>* hostsMAC,
                           const std::list<const char*>* hostsIP) = 0;
    virtual const char* getName (void) const = 0;

protected:
  //  cNetInterface ();
};

#endif /* NETINTERFACE_H_ */
