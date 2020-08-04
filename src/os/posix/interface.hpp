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

#include "protocoltypes.hpp"
#include "ipaddress.hpp"


class cInterface
{
public:
    cInterface(const char* ifname);
    virtual ~cInterface();
    bool open ();
    bool close ();
    bool sendPacket (const uint8_t* payload, size_t length) const;
    bool getMAC (mac_t*);
    bool getIPv4 (cIpAddress&);

private:

    std::string name;
    int ifcHandle;
    int ifIndex;
    mac_t myMac;
    cIpAddress myIP;
};

#endif /* INTERFACE_H_ */

