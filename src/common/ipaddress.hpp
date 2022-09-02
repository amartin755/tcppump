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


#ifndef IPADDRESS_HPP
#define IPADDRESS_HPP

#include <cstring>
#include <string>
#include <cstdint>

#include "inet.h"
#include "bug.hpp"
#include "random.hpp"


class cIPv4
{
public:

    void operator=(const cIPv4&) = delete;       // no copy-assignment operator

    cIPv4 ()
    {
        ipv4.s_addr = 0;
    }
    cIPv4 (const cIPv4& i)
    {
        set (i);
    }
    cIPv4 (const struct in_addr &addr)
    {
        set (addr);
    }
    cIPv4 (const char* ip)
    {
        set (ip);
    }
    cIPv4 (bool randUnicast, bool randMulticast) // construct random IPv4 address
    {
        setRandom (randUnicast, randMulticast);
    }
    void set (const cIPv4& i)
    {
        ipv4 = i.ipv4;
    }
    void set (const struct in_addr &addr)
    {
        ipv4 = addr;
    }
    bool set (const char* ip, size_t len)
    {
        char ipAsString[INET_ADDRSTRLEN];
        if ((len+1) > sizeof(ipAsString))
            return false;
        ::strncpy (ipAsString, ip, len);
        ipAsString[len] = '\0';
        return set (ipAsString);
    }
    bool set (const char* ip)
    {
#if HAVE_PTON
        return !!inet_pton(AF_INET, ip, &ipv4);
#else
        return (ipv4.s_addr = inet_addr (ip)) != INADDR_NONE;
#endif
    }
    void setRandom (bool unicast = true, bool multicast = false)
    {
        BUG_ON (!unicast && !multicast);

        uint32_t ip = cRandom::rand32 ();

        if (unicast && !multicast) // unicast only
        {
            // if random value is multicast, convert it to unicast by clearing the MSB
            if ((ip & 0xF0000000) == 0xE0000000)
                ip &= 0x7FFFFFFF;
        }
        else if (!unicast && multicast) // multicast only
        {
            ip &= 0xEFFFFFFF;
            ip |= 0xE0000000;
        }

        ipv4.s_addr = htonl(ip);
    }
    void clear ()
    {
        ipv4.s_addr = 0;
    }
    struct in_addr get () const
    {
        return ipv4;
    }
    bool get (char* s, size_t len) const
    {
#if HAVE_NTOP
        return !!inet_ntop(AF_INET, &ipv4, s, len);
#else
        std::strncpy (s, inet_ntoa(ipv4), len);
        return true;
#endif
    }
    bool get (std::string &s) const
    {
        char ipAsString[INET_ADDRSTRLEN];
        bool ret = get (ipAsString, sizeof (ipAsString));
        s = ipAsString;
        return ret;
    }
    const uint8_t* getAsArray () const
    {
        return (const uint8_t*)&ipv4.s_addr;
    }

    bool isNull (void) const
    {
        return !ipv4.s_addr;
    }
    bool isMulticast (void) const
    {
        return (ntohl (ipv4.s_addr) & 0xF0000000) == 0xE0000000;
    }
    bool operator ==(const cIPv4 &b) const
    {
        return ipv4.s_addr == b.ipv4.s_addr;
    }
    bool operator !=(const cIPv4 &b) const
    {
        return ipv4.s_addr != b.ipv4.s_addr;
    }
    bool operator< (const cIPv4 &val) const
    {
        return ntohl (ipv4.s_addr) < ntohl (val.ipv4.s_addr);
    }
    bool operator> (const cIPv4 &val) const
    {
        return ntohl (ipv4.s_addr) > ntohl (val.ipv4.s_addr);
    }

#ifdef WITH_UNITTESTS
    static void unitTest ()
    {
        assert (cIPv4() == cIPv4("0.0.0.0"));
        assert (cIPv4() != cIPv4("0.0.0.1"));
        const char x[] = "1.2.3.4dfadfasd";
        cIPv4 a; a.set(x, 7);
        assert (cIPv4("1.2.3.4") == a);
        assert (!a.set("laskdfj"));
        assert (!cIPv4("223.255.255.255").isMulticast());
        assert (cIPv4("224.0.0.0").isMulticast());
        assert (cIPv4("239.255.255.255").isMulticast());
        assert (!cIPv4("240.0.0.0").isMulticast());
        assert (cIPv4("1.2.3.4") < cIPv4("1.3.3.4"));
        assert (cIPv4("2.2.3.4") > cIPv4("1.3.3.4"));
        assert (cIPv4("1.2.3.4").getAsArray()[0] == 1);
        assert (cIPv4("1.2.3.4").getAsArray()[1] == 2);
        assert (cIPv4("1.2.3.4").getAsArray()[2] == 3);
        assert (cIPv4("1.2.3.4").getAsArray()[3] == 4);
        for (int n = 0; n < 10000; n++)
        {
            assert (!cIPv4(true, false).isMulticast());
            assert (cIPv4(false, true).isMulticast());
        }
    }
#endif


private:
    struct in_addr ipv4;
};




#endif /* IPADDRESS_HPP */
