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


#ifndef PARSEHELPER_HPP
#define PARSEHELPER_HPP

#include <cstddef>    // size_t
#include <cstdint>


class cParseHelper
{
public:
    static const char* skipWhitespaces (const char* p);
    static const char* nextKeyStart (const char* p)
    {
        // Keys always start with an alphabetic character
        return nextTokenStart (p, true, false, nullptr);
    }
    static const char* nextKeyEnd (const char* p)
    {
        // alphabetic characters, numbers and '-' are allowed
        return nextTokenEnd (p, true, true, "-");
    }
    static const char* nextValueStart (const char* p)
    {
        // Values always start with an alphabetic character, a number, ", or *
        return nextTokenStart (p, true, true, "\"*");
    }
    static const char* nextValueEnd (const char* p)
    {
        // alphabetic characters, numbers, '.' and ':' are allowed
        return nextTokenEnd (p, true, true, ".:*");
    }
    static const char* nextTokenStart (const char* p, bool isAlpha, bool isDigit, const char* accept);
    static const char* nextTokenEnd (const char* p, bool isAlpha, bool isDigit, const char* accept);
    static const char* nextCharIgnoreWhitspaces (const char* p, char c);
    static int isOneOf (char c, const char* accept);
    static uint8_t* hexStringToBin (const char* hexString, size_t hexStringLen, size_t& binLength);

#ifdef WITH_UNITTESTS
        static void unitTest ();
#endif

};

#endif /* PARSEHELPER_HPP */
