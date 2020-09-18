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


#ifndef TOOLS_HPP_
#define TOOLS_HPP_

#include <cstdint>
#include <cstddef>    // size_t


class Converter
{
public:
    static uint8_t* hexStringToBin (const char* hexString, size_t* binLength);
    static bool hexStringToBin (const char* hexString, size_t hexStringLen, uint8_t* bin, size_t* binLength);

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif
};

#endif /* TOOLS_HPP_ */
