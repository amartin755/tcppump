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

#include "protocoltypes.hpp"
#include "converter.hpp"

/* TODO the whole implementation should be independent of protocoltypes.hpp and converter.hpp
 * The related code must be moved to this file.
 */

class cMacAddress
{
public:
	cMacAddress ()
	{
		mac.set (0);
	}
	cMacAddress (const char* s)
	{
		set (s);
	}
	bool set (const char* s)
	{
		return nn::Converter::stringToMac (s, mac);
	}
	mac_t getRaw ()
	{
		return mac;
	}

private:
	mac_t mac;
};




#endif /* MACADDRESS_HPP */
