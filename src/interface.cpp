/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2019 Andreas Martin (netnag@mailbox.org)
 *
 * interface.cpp
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
#include <cassert>
#include <string>
#include <cstdio>

#include "interface.hpp"


cInterface::cInterface(const char* ifname)
: name (ifname), socket (-1)
{
}

cInterface::~cInterface()
{
	close ();
}

bool cInterface::open ()
{
	// aleady open
	if (socket != -1)
		return true;

	//TODO implement me
	return true;
}

bool cInterface::close ()
{
	// aleady closed
	if (socket == -1)
		return true;

	//TODO implement me
	return true;
}

bool cInterface::sendPacket (const uint8_t* payload, size_t length)
{
	//TODO implement me
	printf ("STUB sendPacket (%zd)\n", length);
	return true;
}

mac_t cInterface::getMAC ()
{
	//TODO implement me
	mac_t x;
	x.a = 0x10;
	x.b = 0x20;
	x.c = 0x30;
	x.d = 0x40;
	x.e = 0x50;
	x.f = 0x60;
	return x;
}
