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


#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>

#include "macaddress.hpp"
#include "ipaddress.hpp"

class cSettings
{
public:
    cSettings ();
    cSettings(const cSettings&) = delete;
    cSettings& operator= (const cSettings&) = delete;

    static cSettings& get(void);

    bool setMyMAC (const char* mac);
    void setMyMAC (const cMacAddress& mac);
    bool setMyIPv4 (const char* ip);
    void setMyIPv4 (const cIPv4& ip);
    bool setMyIPv6 (const char* ip);
    void setMyIPv6 (const cIPv6& ip);
    void setMyMTU (unsigned mtu);
    const cMacAddress& getMyMAC (void) const;
    const cIPv4& getMyIPv4(void) const;
    const cIPv6& getMyIPv6(void) const;
    unsigned getMyMTU (void) const;
    bool isMacSet (void) const {return m_hasMAC;}
    bool isIPSet (void) const {return m_hasIPv4;}
    bool isIPv6Set (void) const {return m_hasIPv6;}
    void setIfName (const char* ifc);
    const std::string getIfName () const;

private:
    bool m_hasMAC;
    bool m_hasIPv4;
    bool m_hasIPv6;
    cMacAddress m_myMAC;
    cIPv4 m_myIP;
    cIPv6 m_myIPv6;
    unsigned m_mtu;
    std::string m_ifName;
};

#endif /* SETTINGS_HPP */
