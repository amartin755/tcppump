/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
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


#ifndef TOOLS_HPP_
#define TOOLS_HPP_

#include <cstdint>
#include "protocoltypes.hpp"

namespace nn
{

	class Converter
	{
	public:
		static uint8_t* hexStringToBin (const char* hexString, int* binLength);
		static bool hexStringToBin (const char* hexString, int hexStringLen, uint8_t* bin, int* binLength);
		static bool stringToMac (const char* str, mac_t& mac, int len = 0);
		static uint16_t getIPv4Chksum (const uint16_t* ipheader, int headerLen);
		static bool stringToIpv4 (const char* str, ipv4_t& ip, int len = 0);

		static void unitTest ();

	private:
		static bool checkMacString (const char* mac, size_t len = 0);
	};

}
#endif /* TOOLS_HPP_ */
