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


#ifndef MACADDRESS_HPP
#define MACADDRESS_HPP

#include <cstdint>
#include <cstring>
#include <cctype>
#include <string>
#include <stdexcept>

#include "bug.hpp"
#include "random.hpp"
#include "parsehelper.hpp"


class cMacAddress
{
public:
    struct mac_t{uint8_t mac[6];};

    cMacAddress ()
    {
        clear();
    }
    cMacAddress (unsigned val)
    {
        set (val);
    }
    cMacAddress (const char* s, size_t len = 0)
    {
        if (!set (s, len))
        {
            clear ();
            throw std::out_of_range ("invalid mac address string");
        }
    }
    cMacAddress (const std::string& s) : cMacAddress (s.c_str(), s.size())
    {        
    }
    cMacAddress (const cMacAddress& obj)
    {
        set (obj);
    }
    cMacAddress (uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5)
    {
        m_mac[0] = b0;
        m_mac[1] = b1;
        m_mac[2] = b2;
        m_mac[3] = b3;
        m_mac[4] = b4;
        m_mac[5] = b5;
    }
    cMacAddress (bool randUnicast, bool randMulticast) // construct random MAC address
    {
        setRandom (randUnicast, randMulticast);
    }
    void operator=(const cMacAddress& obj)
    {
        set (obj);
    }

    void clear ()
    {
        set ((unsigned)0);
    }
    void set (const cMacAddress& obj)
    {
        m_mac[0] = obj.m_mac[0];
        m_mac[1] = obj.m_mac[1];
        m_mac[2] = obj.m_mac[2];
        m_mac[3] = obj.m_mac[3];
        m_mac[4] = obj.m_mac[4];
        m_mac[5] = obj.m_mac[5];
    }
    void set (unsigned val)
    {
        m_mac[0] = m_mac[1] = m_mac[2] = m_mac[3] = m_mac[4] = m_mac[5] = (uint8_t)val;
    }
    bool set (const char* sMac, size_t len = 0)
    {
        if (!len)
            len = std::strlen (sMac);

        std::vector<std::string_view> tokens = cParseHelper::tokenize (sMac, len, ':');

        if (unlikely (tokens.size() != 6))
            return false;

        try
        {
            uint8_t* pMac = (uint8_t*)m_mac;
            for (const auto& token : tokens)
            {
                *pMac++ = cParseHelper::strToUint8 (token.data(), token.size(), 16);
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    void set (const void* b, size_t len)
    {
        BUG_ON (len < size());
        std::memcpy(m_mac, b, size());
    }
    void setAt (int offset, uint8_t value)
    {
        BUG_ON (offset > 5);
        if (offset <= 5)
        {
            m_mac[offset] = value;
        }
    }
    const void* get() const
    {
        return m_mac;
    }
    const uint8_t* getAsArray () const
    {
        return m_mac;
    }
    void get (char* s, size_t len) const
    {
        BUG_ON (len <= MACSTRLEN);
        std::snprintf (s, len, "%02x:%02x:%02x:%02x:%02x:%02x", m_mac[0], m_mac[1], m_mac[2], m_mac[3], m_mac[4], m_mac[5]);
    }
    void get (mac_t* mac) const
    {
        std::memcpy(mac, m_mac, size());
    }
    void get (std::string& s) const
    {
        char cstr[MACSTRLEN+1];
        get (cstr, sizeof(cstr));
        s.assign(cstr);
    }
    void setRandom (bool unicast = true, bool multicast = false)
    {
        BUG_ON (!unicast && !multicast);

        m_mac[0] = cRandom::rand<uint8_t> ();
        m_mac[1] = cRandom::rand<uint8_t> ();
        m_mac[2] = cRandom::rand<uint8_t> ();
        m_mac[3] = cRandom::rand<uint8_t> ();
        m_mac[4] = cRandom::rand<uint8_t> ();
        m_mac[5] = cRandom::rand<uint8_t> ();

        if (unicast && !multicast)
            m_mac[0] &= 0xfe;
        else if (!unicast && multicast)
            m_mac[0] |= 1;
    }
    static constexpr size_t size ()
    {
        return sizeof(m_mac);
    }
    bool isNull () const
    {
        return !m_mac[0] && !m_mac[1] && !m_mac[2] && !m_mac[3] && !m_mac[4] && !m_mac[5];
    }
    bool isBroadcast () const
    {
        return m_mac[0] == 0xffu && m_mac[1] == 0xffu && m_mac[2] == 0xffu && m_mac[3] == 0xffu && m_mac[4] == 0xffu && m_mac[5] == 0xffu;
    }
    bool isMulticast () const
    {
        return (m_mac[0] & 1) && !isBroadcast ();
    }
    bool isUnicast () const
    {
        return !(m_mac[0] & 1);
    }
    bool operator== (const cMacAddress& b) const
    {
        return !std::memcmp (m_mac, b.m_mac, sizeof(m_mac));
    }
    bool operator!= (const cMacAddress& b) const
    {
        return std::memcmp (m_mac, b.m_mac, sizeof(m_mac));
    }


#ifdef WITH_UNITTESTS
    static void unitTest ()
    {
        uint8_t a[] = {1,2,3,4,5,6};
        cMacAddress b("01:02:03:04:05:06");
        BUG_IF_NOT (b.size() == 6);
        BUG_IF_NOT (!std::memcmp(a, b.m_mac, sizeof(a)));

        BUG_IF_NOT (cMacAddress().isNull());
        BUG_IF_NOT (cMacAddress("ff:ff:ff:ff:ff:ff").isBroadcast());
        BUG_IF_NOT (!cMacAddress("ff:ff:ff:ff:ff:ff").isMulticast());
        BUG_IF_NOT (cMacAddress("01:ff:ff:ff:ff:ff").isMulticast());
        BUG_IF_NOT (!cMacAddress("80:ff:ff:ff:ff:ff").isMulticast());

        BUG_ON (b.set ("11:22:33:44:55:66:77"));
        BUG_ON (b.set ("11:a2:3g:44:55:66"));
        BUG_ON (b.set ("11:a2:g:44:55:66"));
        BUG_ON (b.set ("11:a2:g3:44:55:66"));
        BUG_ON (b.set (""));
        BUG_ON (b.set ("11:a2:3344:55:66"));
        BUG_ON (b.set ("11:a2:33:55:66"));
        BUG_ON (b.set ("11:a2:33::44:55:66"));
        BUG_ON (b.set ("11:a2:33::44:55"));
        BUG_ON (b.set ("11:a2:33:44:55:"));
        BUG_ON (b.set ("11:a2:33::44:55:66:"));
        BUG_ON (b.set (":a2:33::44:55:66"));
        BUG_ON (b.set (":a2:33::44:55:66:77"));

        BUG_ON (!b.set ("11:a2:33:44:55:66"));
        BUG_ON (b.m_mac[0] != 0x11 || b.m_mac[1] != 0xa2 || b.m_mac[2] != 0x33 || b.m_mac[3] != 0x44 || b.m_mac[4] != 0x55 || b.m_mac[5] != 0x66);
        BUG_ON (!b.set ("11:2:33:44:55:66"));
        BUG_ON (b.m_mac[0] != 0x11 || b.m_mac[1] != 0x2 || b.m_mac[2] != 0x33 || b.m_mac[3] != 0x44 || b.m_mac[4] != 0x55 || b.m_mac[5] != 0x66);
        BUG_ON (!b.set ("0:1:2:3:4:5"));
        BUG_ON (b.m_mac[0] != 0 || b.m_mac[1] != 1 || b.m_mac[2] != 2 || b.m_mac[3] != 3 || b.m_mac[4] != 4 || b.m_mac[5] != 5);
    }
#endif

private:
    static const int MACSTRLEN = 17;

    uint8_t m_mac[6];
};




#endif /* MACADDRESS_HPP */
