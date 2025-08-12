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

        if (len == 1 && *sMac == '*')
        {
            setRandom ();
            return true;
        }
        if (len < 11)
            return false;

        // collect all tokens
        const char* tokens[] = {sMac, nullptr, nullptr, nullptr, nullptr, nullptr, sMac + len};
        int t = 1;
        const char* p = sMac;
        for (size_t n = 0; n < len; n++)
        {
            if (*p++ == ':' && n != 0)
                tokens[t++] = p;
            if (t > (int)sizeof (mac_t))
                return false;
        }
        if (t < (int)sizeof (mac_t))
            return false;

        // parse tokens
        uint8_t newMac[sizeof (mac_t)];
        for (int n = 0; n < t; n++)
        {
            // only the first three tokens end with '.'
            size_t tokLen = n == t-1 ? tokens[n+1] - tokens[n] : tokens[n+1] - tokens[n] - 1;

            if (tokLen == 0)
                return false;

            // random number?
            if (*tokens[n] == '*')
            {
                if (tokLen == 1)
                    newMac[n] = cRandom::rand8 ();
                else
                {
                    // random number with specified range?
                    uint64_t min, max;
                    if (!cParseHelper::range (tokens[n] + 1, tokLen - 1, 16, min, max))
                        return false;
                    if (min > 255 || max > 255)
                        return false;
                    newMac[n] = cRandom::rand8 ((uint8_t)min, (uint8_t)max);
                }
            }
            else
            {
                try
                {
                    newMac[n] = cParseHelper::strToUint8 (tokens[n], tokLen, 16);
                }
                catch(...)
                {
                    return false;
                }
            }
        }
        static_assert (sizeof (newMac) == sizeof (m_mac), "");
        std::memcpy (&m_mac, newMac, sizeof (m_mac));
        return true;
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

        m_mac[0] = cRandom::rand8 ();
        m_mac[1] = cRandom::rand8 ();
        m_mac[2] = cRandom::rand8 ();
        m_mac[3] = cRandom::rand8 ();
        m_mac[4] = cRandom::rand8 ();
        m_mac[5] = cRandom::rand8 ();

        if (unicast && !multicast)
            m_mac[0] &= 0xfe;
        else if (!unicast && multicast)
            m_mac[0] |= 1;
    }
    static size_t size ()
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


#ifdef WITH_UNITTESTS
    static void unitTest ()
    {
        BUG_IF_NOT (!isValidString(""));
        BUG_IF_NOT (!isValidString("11:22:33:44:55:66:77"));
        BUG_IF_NOT (isValidString("11:22:33:44:55:66"));
        BUG_IF_NOT (isValidString("11:a2:33:44:55:66"));
        BUG_IF_NOT (!isValidString("11:a2:3g:44:55:66"));
        BUG_IF_NOT (!isValidString("11:a2:g3:44:55:66"));

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
        BUG_ON (!b.set ("11:a2:33:44:55:66"));
        BUG_ON (b.set ("11:a2:3g:44:55:66"));
        BUG_ON (!b.set ("00:*[1-2]:02:*[1-2]:04:*[1-2]"));
        BUG_ON (b.m_mac[0] != 0 || b.m_mac[2] != 2 || b.m_mac[4] != 4);
        BUG_ON (b.m_mac[1] != 1 && b.m_mac[1] != 2);
        BUG_ON (b.m_mac[3] != 1 && b.m_mac[3] != 2);
        BUG_ON (b.m_mac[5] != 1 && b.m_mac[5] != 2);
        BUG_ON (!b.set ("00:*[aa-ab]:02:*[cc-cd]:04:*[ee-ef]"));
        BUG_ON (b.m_mac[0] != 0 || b.m_mac[2] != 2 || b.m_mac[4] != 4);
        BUG_ON (b.m_mac[1] != 0xaa && b.m_mac[1] != 0xab);
        BUG_ON (b.m_mac[3] != 0xcc && b.m_mac[3] != 0xcd);
        BUG_ON (b.m_mac[5] != 0xee && b.m_mac[5] != 0xef);
        BUG_ON (b.set ("00:*[aa-100]:02:*[cc-cd]:04:*[ee-ef]"));
    }
#endif

private:
    static bool isValidString (const char* mac, size_t len = 0)
    {
        if (!len)
            len = std::strlen (mac);

        // 11:22:33:44:55:66 or 11-22-33-44-55-66

        if (len != MACSTRLEN)
            return false;

        for (size_t n = 0; n < len; n += 3)
        {
            if (!std::isxdigit (mac[n]) || !std::isxdigit (mac[n + 1]))
                return false;
            if ((n + 2) < len)
                if (mac[n + 2] != ':')
                    return false;
        }

        return true;
    }

    static const int MACSTRLEN = 17;

    uint8_t m_mac[6];
};




#endif /* MACADDRESS_HPP */
