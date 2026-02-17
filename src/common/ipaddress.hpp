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
#include <stdexcept>

#include "inet.h"
#include "bug.hpp"
#include "random.hpp"
#include "parsehelper.hpp"


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
        if (!set (ip))
            throw std::out_of_range ("invalid ipv4 address string");
    }
    cIPv4 (const char* ip, size_t len)
    {
        if (!set (ip, len))
            throw std::out_of_range ("invalid ipv4 address string");
    }
    cIPv4 (const std::string& s) : cIPv4 (s.c_str())
    {        
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
        std::memcpy (ipAsString, ip, len);
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
    void setAt (int offset, uint8_t value)
    {
        if (likely(offset <= 3))
        {
            uint8_t* ip = (uint8_t*)&ipv4.s_addr;
            ip[offset] = value;
        }
    }
    static constexpr size_t size ()
    {
        return sizeof(ipv4);
    }
    const cIPv4& setRandom ()
    {
        uint32_t ip = cRandom::rand<uint32_t> ();

        // if random value is multicast, convert it to unicast by clearing the MSB
        if ((ip & 0xF0000000) == 0xE0000000)
            ip &= 0x7FFFFFFF;

        ipv4.s_addr = htonl(ip);
        return *this;
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
        BUG_IF_NOT (cIPv4() == cIPv4("0.0.0.0"));
        BUG_IF_NOT (cIPv4() != cIPv4("0.0.0.1"));
        const char x[] = "1.2.3.4dfadfasd";
        cIPv4 a; a.set(x, 7);
        BUG_IF_NOT (cIPv4("1.2.3.4") == a);
        a.clear ();
        BUG_ON (a.get().s_addr);
        BUG_IF_NOT (!a.set("laskdfj"));
        BUG_IF_NOT (!cIPv4("223.255.255.255").isMulticast());
        BUG_IF_NOT (cIPv4("224.0.0.0").isMulticast());
        BUG_IF_NOT (cIPv4("239.255.255.255").isMulticast());
        BUG_IF_NOT (!cIPv4("240.0.0.0").isMulticast());
        BUG_IF_NOT (cIPv4("1.2.3.4") < cIPv4("1.3.3.4"));
        BUG_IF_NOT (cIPv4("2.2.3.4") > cIPv4("1.3.3.4"));
        BUG_IF_NOT (cIPv4("1.2.3.4").getAsArray()[0] == 1);
        BUG_IF_NOT (cIPv4("1.2.3.4").getAsArray()[1] == 2);
        BUG_IF_NOT (cIPv4("1.2.3.4").getAsArray()[2] == 3);
        BUG_IF_NOT (cIPv4("1.2.3.4").getAsArray()[3] == 4);
        for (int n = 0; n < 10000; n++)
        {
            BUG_IF_NOT (cIPv4().setRandom() != cIPv4().setRandom());
        }

        BUG_ON (a.set (""));
        BUG_ON (a.set ("1"));
        BUG_ON (a.set ("."));
        BUG_ON (a.set ("..1."));
        BUG_ON (a.set ("...."));
        BUG_ON (a.set ("a.b.c.d"));
        BUG_ON (a.set ("1.2"));
        BUG_ON (a.set ("1.2.3."));
        BUG_ON (a.set ("1.2.3.4."));
        BUG_ON (a.set ("1.2..3.4"));
        BUG_ON (a.set ("1.2.3.."));
        BUG_ON (a.set ("1.*.3."));
        BUG_ON (a.set ("256.2.3.4"));

        {
            cIPv4 x ("1.2.3.4");
            x.setAt (0, 10);
            BUG_ON (x.getAsArray()[0] != 10);
            BUG_ON (x.getAsArray()[1] != 2);
            BUG_ON (x.getAsArray()[2] != 3);
            BUG_ON (x.getAsArray()[3] != 4);
            x.setAt (2, 30);
            BUG_ON (x.getAsArray()[0] != 10);
            BUG_ON (x.getAsArray()[1] != 2);
            BUG_ON (x.getAsArray()[2] != 30);
            BUG_ON (x.getAsArray()[3] != 4);
            x.setAt (1, 20);
            BUG_ON (x.getAsArray()[0] != 10);
            BUG_ON (x.getAsArray()[1] != 20);
            BUG_ON (x.getAsArray()[2] != 30);
            BUG_ON (x.getAsArray()[3] != 4);
            x.setAt (3, 40);
            BUG_ON (x.getAsArray()[0] != 10);
            BUG_ON (x.getAsArray()[1] != 20);
            BUG_ON (x.getAsArray()[2] != 30);
            BUG_ON (x.getAsArray()[3] != 40);
        }
    }
#endif


private:
    struct in_addr ipv4;
};

class cIPv6
{
public:

    void operator=(const cIPv6&) = delete;       // no copy-assignment operator

    cIPv6 ()
    {
        clear ();
    }
    cIPv6 (const cIPv6& i)
    {
        set (i);
    }
    cIPv6 (const struct in6_addr &addr)
    {
        set (addr);
    }
    cIPv6 (const char* ip)
    {
        if (!set (ip))
            throw std::out_of_range ("invalid ipv6 address string");
    }
    cIPv6 (const char* ip, size_t len)
    {
        if (!set (ip, len))
            throw std::out_of_range ("invalid ipv6 address string");
    }
    cIPv6 (const std::string& s) : cIPv6 (s.c_str())
    {        
    }
    void set (const cIPv6& i)
    {
        ipv6 = i.ipv6;
    }
    void set (const struct in6_addr &addr)
    {
        ipv6 = addr;
    }
    bool set (const char* ip, size_t len)
    {
        char ipAsString[INET6_ADDRSTRLEN];
        if ((len+1) > sizeof(ipAsString))
            return false;
        std::memcpy (ipAsString, ip, len);
        ipAsString[len] = '\0';
        return set (ipAsString);
    }
    void setAt (int offset, uint16_t value)
    {
        if (likely(offset <= 7))
        {
            uint16_t* ip = (uint16_t*)&ipv6.s6_addr[0];
            ip[offset] = htons(value);
        }
    }
    bool set (const char* ip)
    {
#if HAVE_PTON
        return !!inet_pton(AF_INET6, ip, &ipv6);
#else
        return (ipv6.s_addr = inet_addr (ip)) != INADDR_NONE;
#endif
    }
    static constexpr size_t size ()
    {
        return sizeof(ipv6);
    }
    const cIPv6& setRandom ()
    {
        // FIXME this is very inefficient and makes only sense for predictable randoms
        uint16_t r = cRandom::rand<uint16_t> ();
        setAt (0, r);
        r = cRandom::rand<uint16_t> ();
        setAt (1, r);
        r = cRandom::rand<uint16_t> ();
        setAt (2, r);
        r = cRandom::rand<uint16_t> ();
        setAt (3, r);
        r = cRandom::rand<uint16_t> ();
        setAt (4, r);
        r = cRandom::rand<uint16_t> ();
        setAt (5, r);
        r = cRandom::rand<uint16_t> ();
        setAt (6, r);
        r = cRandom::rand<uint16_t> ();
        setAt (7, r);

        // if random value is multicast, convert it to unicast
        if (ipv6.s6_addr[0] == 0xff)
            ipv6.s6_addr[0] &= 0x3f;

        return *this;
    }
    void clear ()
    {
        memset (&ipv6, 0, sizeof(ipv6));
    }
    struct in6_addr get () const
    {
        return ipv6;
    }
    bool get (char* s, size_t len) const
    {
#if HAVE_NTOP
        return !!inet_ntop(AF_INET6, &ipv6, s, len);
#else
        std::strncpy (s, inet_ntoa(ipv6), len);
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
        return (const uint8_t*)&ipv6.s6_addr[0];
    }

    bool isNull (void) const
    {
        return !ipv6.s6_addr[0] &&
               !ipv6.s6_addr[1] &&
               !ipv6.s6_addr[2] &&
               !ipv6.s6_addr[3] &&
               !ipv6.s6_addr[4] &&
               !ipv6.s6_addr[5] &&
               !ipv6.s6_addr[6] &&
               !ipv6.s6_addr[7] &&
               !ipv6.s6_addr[8] &&
               !ipv6.s6_addr[9] &&
               !ipv6.s6_addr[10] &&
               !ipv6.s6_addr[11] &&
               !ipv6.s6_addr[12] &&
               !ipv6.s6_addr[13] &&
               !ipv6.s6_addr[14] &&
               !ipv6.s6_addr[15];
    }
    bool isMulticast (void) const
    {
        return ipv6.s6_addr[0] == 0xff;
    }
    bool operator ==(const cIPv6 &b) const
    {
        return !std::memcmp (&ipv6, &b.ipv6, sizeof (ipv6));
    }
    bool operator !=(const cIPv6 &b) const
    {
        return std::memcmp (&ipv6, &b.ipv6, sizeof (ipv6));
    }
    bool operator< (const cIPv6 &val) const
    {
        return std::memcmp (&ipv6, &val.ipv6, sizeof (ipv6)) < 0;
    }
    bool operator> (const cIPv6 &val) const
    {
        return std::memcmp (&ipv6, &val.ipv6, sizeof (ipv6)) > 0;
    }

#ifdef WITH_UNITTESTS
    static void unitTest ()
    {
        BUG_IF_NOT (cIPv6() == cIPv6("::"));
        BUG_IF_NOT (cIPv6() != cIPv6("::1"));
        const char x[] = "fe80::1ff:fe23:4567:890adfadfasd";
        cIPv6 a; a.set(x, 24);
        BUG_IF_NOT (cIPv6("fe80::1ff:fe23:4567:890a") == a);
        BUG_IF_NOT (!a.set("laskdfj"));
        BUG_IF_NOT (!cIPv6("fe80::1ff:fe23:4567:890a").isMulticast());
        BUG_IF_NOT (cIPv6("ff02::6").isMulticast());
        BUG_IF_NOT (cIPv6("fe80::1ff:fe23:4567:890a") < cIPv6("fe80::1ff:fe23:4577:890a"));
        BUG_IF_NOT (cIPv6("fe80::1ff:fe33:4567:890a") > cIPv6("fe80::1ff:fe23:4577:890a"));
        BUG_IF_NOT (cIPv6("fe80::1ff:fe33:4567:890a") != cIPv6("fe80::1ff:fe23:4577:890a"));
        BUG_IF_NOT (a.getAsArray()[0] == 0xfe);
        BUG_IF_NOT (a.getAsArray()[1] == 0x80);
        BUG_IF_NOT (a.getAsArray()[2] == 0);
        BUG_IF_NOT (a.getAsArray()[3] == 0);
        BUG_IF_NOT (a.getAsArray()[4] == 0);
        BUG_IF_NOT (a.getAsArray()[5] == 0);
        BUG_IF_NOT (a.getAsArray()[6] == 0);
        BUG_IF_NOT (a.getAsArray()[7] == 0);
        BUG_IF_NOT (a.getAsArray()[8] == 0x01);
        BUG_IF_NOT (a.getAsArray()[9] == 0xff);
        BUG_IF_NOT (a.getAsArray()[10] == 0xfe);
        BUG_IF_NOT (a.getAsArray()[11] == 0x23);
        BUG_IF_NOT (a.getAsArray()[12] == 0x45);
        BUG_IF_NOT (a.getAsArray()[13] == 0x67);
        BUG_IF_NOT (a.getAsArray()[14] == 0x89);
        BUG_IF_NOT (a.getAsArray()[15] == 0x0a);
        for (int n = 0; n < 10000; n++)
        {
            BUG_IF_NOT (cIPv6().setRandom() != cIPv6().setRandom());
        }

        {
            cIPv6 x ("1000:2000:3000:4000:5000:6000:7000:8000");
            x.setAt (0, 0x1002);
            BUG_ON (((uint16_t*)&x.ipv6)[0] != htons(0x1002));
            BUG_ON (((uint16_t*)&x.ipv6)[1] != htons(0x2000));
            BUG_ON (((uint16_t*)&x.ipv6)[2] != htons(0x3000));
            BUG_ON (((uint16_t*)&x.ipv6)[3] != htons(0x4000));
            BUG_ON (((uint16_t*)&x.ipv6)[4] != htons(0x5000));
            BUG_ON (((uint16_t*)&x.ipv6)[5] != htons(0x6000));
            BUG_ON (((uint16_t*)&x.ipv6)[6] != htons(0x7000));
            BUG_ON (((uint16_t*)&x.ipv6)[7] != htons(0x8000));
            x.setAt (2, 0x3004);
            BUG_ON (((uint16_t*)&x.ipv6)[0] != htons(0x1002));
            BUG_ON (((uint16_t*)&x.ipv6)[1] != htons(0x2000));
            BUG_ON (((uint16_t*)&x.ipv6)[2] != htons(0x3004));
            BUG_ON (((uint16_t*)&x.ipv6)[3] != htons(0x4000));
            BUG_ON (((uint16_t*)&x.ipv6)[4] != htons(0x5000));
            BUG_ON (((uint16_t*)&x.ipv6)[5] != htons(0x6000));
            BUG_ON (((uint16_t*)&x.ipv6)[6] != htons(0x7000));
            BUG_ON (((uint16_t*)&x.ipv6)[7] != htons(0x8000));
            x.setAt (1, 0x2003);
            BUG_ON (((uint16_t*)&x.ipv6)[0] != htons(0x1002));
            BUG_ON (((uint16_t*)&x.ipv6)[1] != htons(0x2003));
            BUG_ON (((uint16_t*)&x.ipv6)[2] != htons(0x3004));
            BUG_ON (((uint16_t*)&x.ipv6)[3] != htons(0x4000));
            BUG_ON (((uint16_t*)&x.ipv6)[4] != htons(0x5000));
            BUG_ON (((uint16_t*)&x.ipv6)[5] != htons(0x6000));
            BUG_ON (((uint16_t*)&x.ipv6)[6] != htons(0x7000));
            BUG_ON (((uint16_t*)&x.ipv6)[7] != htons(0x8000));
            x.setAt (3, 0x4005);
            BUG_ON (((uint16_t*)&x.ipv6)[0] != htons(0x1002));
            BUG_ON (((uint16_t*)&x.ipv6)[1] != htons(0x2003));
            BUG_ON (((uint16_t*)&x.ipv6)[2] != htons(0x3004));
            BUG_ON (((uint16_t*)&x.ipv6)[3] != htons(0x4005));
            BUG_ON (((uint16_t*)&x.ipv6)[4] != htons(0x5000));
            BUG_ON (((uint16_t*)&x.ipv6)[5] != htons(0x6000));
            BUG_ON (((uint16_t*)&x.ipv6)[6] != htons(0x7000));
            BUG_ON (((uint16_t*)&x.ipv6)[7] != htons(0x8000));
            x.setAt (4, 0x5006);
            BUG_ON (((uint16_t*)&x.ipv6)[0] != htons(0x1002));
            BUG_ON (((uint16_t*)&x.ipv6)[1] != htons(0x2003));
            BUG_ON (((uint16_t*)&x.ipv6)[2] != htons(0x3004));
            BUG_ON (((uint16_t*)&x.ipv6)[3] != htons(0x4005));
            BUG_ON (((uint16_t*)&x.ipv6)[4] != htons(0x5006));
            BUG_ON (((uint16_t*)&x.ipv6)[5] != htons(0x6000));
            BUG_ON (((uint16_t*)&x.ipv6)[6] != htons(0x7000));
            BUG_ON (((uint16_t*)&x.ipv6)[7] != htons(0x8000));
            x.setAt (5, 0x6007);
            BUG_ON (((uint16_t*)&x.ipv6)[0] != htons(0x1002));
            BUG_ON (((uint16_t*)&x.ipv6)[1] != htons(0x2003));
            BUG_ON (((uint16_t*)&x.ipv6)[2] != htons(0x3004));
            BUG_ON (((uint16_t*)&x.ipv6)[3] != htons(0x4005));
            BUG_ON (((uint16_t*)&x.ipv6)[4] != htons(0x5006));
            BUG_ON (((uint16_t*)&x.ipv6)[5] != htons(0x6007));
            BUG_ON (((uint16_t*)&x.ipv6)[6] != htons(0x7000));
            BUG_ON (((uint16_t*)&x.ipv6)[7] != htons(0x8000));
            x.setAt (6, 0x7008);
            BUG_ON (((uint16_t*)&x.ipv6)[0] != htons(0x1002));
            BUG_ON (((uint16_t*)&x.ipv6)[1] != htons(0x2003));
            BUG_ON (((uint16_t*)&x.ipv6)[2] != htons(0x3004));
            BUG_ON (((uint16_t*)&x.ipv6)[3] != htons(0x4005));
            BUG_ON (((uint16_t*)&x.ipv6)[4] != htons(0x5006));
            BUG_ON (((uint16_t*)&x.ipv6)[5] != htons(0x6007));
            BUG_ON (((uint16_t*)&x.ipv6)[6] != htons(0x7008));
            BUG_ON (((uint16_t*)&x.ipv6)[7] != htons(0x8000));
            x.setAt (7, 0x8009);
            BUG_ON (((uint16_t*)&x.ipv6)[0] != htons(0x1002));
            BUG_ON (((uint16_t*)&x.ipv6)[1] != htons(0x2003));
            BUG_ON (((uint16_t*)&x.ipv6)[2] != htons(0x3004));
            BUG_ON (((uint16_t*)&x.ipv6)[3] != htons(0x4005));
            BUG_ON (((uint16_t*)&x.ipv6)[4] != htons(0x5006));
            BUG_ON (((uint16_t*)&x.ipv6)[5] != htons(0x6007));
            BUG_ON (((uint16_t*)&x.ipv6)[6] != htons(0x7008));
            BUG_ON (((uint16_t*)&x.ipv6)[7] != htons(0x8009));
        }

    }
#endif


private:
    struct in6_addr ipv6;
};



#endif /* IPADDRESS_HPP */
