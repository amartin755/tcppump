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

#include <stdexcept>

#include "settings.hpp"
#include "ethernetpacket.hpp"

static cSettings globalSettings;

cSettings::cSettings () : hasMAC(false), hasIPv4(false), mtu(cEthernetPacket::MAX_ETHERNET_PAYLOAD)
{

}

cSettings& cSettings::get(void)
{
    return globalSettings;
}

bool cSettings::setMyMAC (const char* mac)
{
    hasMAC = myMAC.set (mac);
    return hasMAC;
}

void cSettings::setMyMAC (const cMacAddress& mac)
{
    hasMAC = true;
    myMAC.set (mac);
}

bool cSettings::setMyIPv4 (const char* ip)
{
    hasIPv4 = myIP.set(ip);
    return hasIPv4;
}

void cSettings::setMyIPv4 (const cIpAddress& ip)
{
    hasIPv4 = true;
    myIP.set(ip);
}

void cSettings::setMyMTU (unsigned mtu)
{
    this->mtu = mtu;
}

const cMacAddress& cSettings::getMyMAC(void) const
{
    if (!hasMAC)
        throw std::runtime_error("Could not determine mac address of interface.\n"
                "Please assign an MAC address to the network interface or use the parameter --mymac.");

    return myMAC;
}

const cIpAddress& cSettings::getMyIPv4(void) const
{
    if (!hasIPv4)
        throw std::runtime_error("Could not determine IPv4 address of interface.\n"
                "Please assign an IP address to the network interface or use the parameter --myip4.");

    return myIP;
}

unsigned cSettings::getMyMTU (void) const
{
    return mtu;
}
