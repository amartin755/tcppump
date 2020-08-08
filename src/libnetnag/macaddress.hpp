/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2020 Andreas Martin (netnag@mailbox.org)
 *
 * macaddress.hpp
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

#ifndef MACADDRESS_HPP
#define MACADDRESS_HPP

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <string>

class cMacAddress
{
public:
    typedef struct {uint8_t mac[6];} mac_t;

    cMacAddress ()
	{
		clear();
	}
    cMacAddress (unsigned val)
	{
		set (val);
	}
	cMacAddress (const char* s)
	{
		set (s);
	}
	void clear ()
	{
		set ((unsigned)0);
	}
	void set (const cMacAddress& obj)
	{
		mac[0] = obj.mac[0];
		mac[1] = obj.mac[1];
		mac[2] = obj.mac[2];
		mac[3] = obj.mac[3];
		mac[4] = obj.mac[4];
		mac[5] = obj.mac[5];
	}
	void set (unsigned val)
	{
		mac[0] = mac[1] = mac[2] = mac[3] = mac[4] = mac[5] = (uint8_t)val;
	}
	bool set (const char* s, size_t len = 0)
	{
	    if (!isValidString (s, len))
	        return false;

	    uint8_t* pMac = (uint8_t*)mac;
	    for (int n = 0; n < 17; n += 3)
	    {
	        *pMac++ = (uint8_t)strtoul (&s[n], NULL, 16);
	    }

	    return true;
	}
	void set (const void* b, size_t len)
	{
		assert (len >= size());
		::memcpy(mac, b, size());
	}
	const void* get() const
	{
		return mac;
	}
	void get (char* s, size_t len) const
	{
		assert (len > 17);
		::snprintf (s, len, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	void get (mac_t* mac) const
	{
		::memcpy(mac, this->mac, size());
	}
	void get (std::string& s) const
	{
		char cstr[17+1];
		get (cstr, sizeof(cstr));
		s.assign(cstr);
	}

	static size_t size ()
	{
		return sizeof(mac);
	}
    bool isNull () const
    {
        return !mac[0] && !mac[1] && !mac[2] && !mac[3] && !mac[4] && !mac[5];
    }
    bool isBroadcast () const
    {
        return mac[0] == 0xffu && mac[1] == 0xffu && mac[2] == 0xffu && mac[3] == 0xffu && mac[4] == 0xffu && mac[5] == 0xffu;
    }
    bool isMulticast () const
    {
        return (mac[0] & 1) && !isBroadcast ();
    }


#ifdef WITH_UNITTESTS
	static void unitTest ()
	{
	    assert (!isValidString(""));
	    assert (!isValidString("11:22:33:44:55:66:77"));
	    assert (isValidString("11:22:33:44:55:66"));
	    assert (isValidString("11-22-33-44-55-66"));
	    assert (isValidString("11:a2:33:44:55:66"));
	    assert (!isValidString("11:a2:3g:44:55:66"));
	    assert (!isValidString("11:a2:g3:44:55:66"));

	    uint8_t a[] = {1,2,3,4,5,6};
	    cMacAddress b("01:02:03:04:05:06");
	    assert (b.size() == 6);
	    assert (!::memcmp(a, b.mac, sizeof(a)));

	    assert (cMacAddress().isNull());
	    assert (cMacAddress("ff:ff:ff:ff:ff:ff").isBroadcast());
	    assert (!cMacAddress("ff:ff:ff:ff:ff:ff").isMulticast());
	    assert (cMacAddress("01:ff:ff:ff:ff:ff").isMulticast());
	    assert (!cMacAddress("80:ff:ff:ff:ff:ff").isMulticast());
	}
#endif

private:
    static bool isValidString (const char* mac, size_t len = 0)
    {
        if (!len)
            len = ::strlen (mac);

        // 11:22:33:44:55:66 or 11-22-33-44-55-66

        if (len != 17)
            return false;

        for (size_t n = 0; n < len; n += 3)
        {
            if (!isxdigit (mac[n]) || !isxdigit (mac[n + 1]))
                return false;
            if ((n + 2) < len)
                if (mac[n + 2] != '-' && mac[n + 2] != ':')
                    return false;
        }

        return true;
    }


    uint8_t mac[6];
};




#endif /* MACADDRESS_HPP */