// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
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

cSettings::cSettings () : m_hasMAC(false), m_hasIPv4(false), m_hasIPv6(false), m_mtu(cEthernetPacket::MAX_ETHERNET_PAYLOAD)
{

}

cSettings& cSettings::get(void)
{
    return globalSettings;
}

bool cSettings::setMyMAC (const char* mac)
{
    m_hasMAC = m_myMAC.set (mac);
    return m_hasMAC;
}

void cSettings::setMyMAC (const cMacAddress& mac)
{
    m_hasMAC = true;
    m_myMAC.set (mac);
}

bool cSettings::setMyIPv4 (const char* ip)
{
    m_hasIPv4 = m_myIP.set(ip);
    return m_hasIPv4;
}

void cSettings::setMyIPv4 (const cIPv4& ip)
{
    m_hasIPv4 = true;
    m_myIP.set(ip);
}

bool cSettings::setMyIPv6 (const char* ip)
{
    m_hasIPv6 = m_myIPv6.set(ip);
    return m_hasIPv6;
}

void cSettings::setMyIPv6 (const cIPv6& ip)
{
    m_hasIPv6 = true;
    m_myIPv6.set(ip);
}

void cSettings::setMyMTU (unsigned mtu)
{
    m_mtu = mtu;
}

const cMacAddress& cSettings::getMyMAC(void) const
{
    if (!m_hasMAC)
        throw std::runtime_error("Could not determine mac address of interface.\n"
                "Please assign an MAC address to the network interface or use the parameter --mymac.");

    return m_myMAC;
}

const cIPv4& cSettings::getMyIPv4(void) const
{
    if (!m_hasIPv4)
        throw std::runtime_error("Could not determine IPv4 address of interface.\n"
                "Please assign an IP address to the network interface or use the parameter --myip4.");

    return m_myIP;
}

const cIPv6& cSettings::getMyIPv6(void) const
{
    if (!m_hasIPv6)
        throw std::runtime_error("Could not determine IPv6 address of interface.\n"
                "Please assign an IP address to the network interface or use the parameter --myip6.");

    return m_myIPv6;
}

unsigned cSettings::getMyMTU (void) const
{
    return m_mtu;
}

void cSettings::setIfName (const char* ifc)
{
    m_ifName = ifc;
}

const std::string cSettings::getIfName () const
{
    if (!m_ifName.size())
        throw std::runtime_error("Could not determine network interface name.\n"
                "Please set parameter -i.");
    return m_ifName;
}
