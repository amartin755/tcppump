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


#ifndef IPADDRESS_HPP
#define IPADDRESS_HPP

#include <cstring>
#include <string>

#include "inet.h"


class cIpAddress
{
public:

    void operator=(const cIpAddress&) = delete;       // no copy-assignment operator

	cIpAddress ()
	{
		ipv4.s_addr = 0;
	}
	cIpAddress (const cIpAddress& i)
	{
		set (i);
	}
	cIpAddress (const struct in_addr &addr)
	{
		set (addr);
	}
	cIpAddress (const char* ip)
	{
		set (ip);
	}
	void set (const cIpAddress& i)
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
	struct in_addr get () const
	{
		return ipv4;
	}
	bool get (char* s, size_t len) const
	{
#if HAVE_NTOP
		return !!inet_ntop(AF_INET, &ipv4, s, len);
#else
		::strncpy (s, inet_ntoa(ipv4), len);
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

	bool isNull (void) const
	{
		return !ipv4.s_addr;
	}
	bool operator ==(const cIpAddress &b) const
	{
		return ipv4.s_addr == b.ipv4.s_addr;
	}
	bool operator !=(const cIpAddress &b) const
	{
		return ipv4.s_addr != b.ipv4.s_addr;
	}

#ifdef WITH_UNITTESTS
	static void unitTest ()
	{
		assert (cIpAddress() == cIpAddress("0.0.0.0"));
		assert (cIpAddress() != cIpAddress("0.0.0.1"));
		const char x[] = "1.2.3.4dfadfasd";
		cIpAddress a; a.set(x, 7);
		assert (cIpAddress("1.2.3.4") == a);
		assert (!a.set("laskdfj"));
	}
#endif


private:
	struct in_addr ipv4;
};




#endif /* IPADDRESS_HPP */
