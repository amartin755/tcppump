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


#ifndef PARSEHELPER_HPP
#define PARSEHELPER_HPP

#include <cstddef>    // size_t
#include <cstdint>
#include <stdexcept>
#include <cerrno>
#include <stdexcept>


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
        // Values always start with an alphabetic character, a number, ", < or *
        return nextTokenStart (p, true, true, "\"*<");
    }
    static const char* nextValueEnd (const char* p)
    {
        // alphabetic characters, numbers, '.' and ':' are allowed
        return nextTokenEnd (p, true, true, ".:*[-]");
    }
    static const char* nextTokenStart (const char* p, bool isAlpha, bool isDigit, const char* accept);
    static const char* nextTokenEnd (const char* p, bool isAlpha, bool isDigit, const char* accept);
    static const char* nextCharIgnoreWhitspaces (const char* p, char c);
    static int isOneOf (char c, const char* accept);
    static uint8_t* hexStringToBin (const char* hexString, size_t hexStringLen, size_t& binLength);
    static bool range (const char* p, size_t len, int base, uint64_t& begin, uint64_t& end);
    static uint8_t strToUint8 (const char* p, size_t len, int base)
    {
        if (len > 4) // max val: 255 or 0xff --> 4
            throw std::out_of_range ("string too long");

        char str[5];
        std::memcpy (str, p, len);
        str[len] = '\0';

        char* end;
        unsigned long val = strtoul (str, &end, base);
        if (end != &str[len] || errno == ERANGE || val > 255)
            throw std::out_of_range ("value > 255");

        return (uint8_t)val;
    }
    static unsigned hexCharToInt (char c)
    {
        switch (c)
        {
            case '0':
                return 0;
            case '1':
                return 1;
            case '2':
                return 2;
            case '3':
                return 3;
            case '4':
                return 4;
            case '5':
                return 5;
            case '6':
                return 6;
            case '7':
                return 7;
            case '8':
                return 8;
            case '9':
                return 9;
            case 'a':
            case 'A':
                return 10;
            case 'b':
            case 'B':
                return 11;
            case 'c':
            case 'C':
                return 12;
            case 'd':
            case 'D':
                return 13;
            case 'e':
            case 'E':
                return 14;
            case 'f':
            case 'F':
                return 15;
        }
        throw std::out_of_range ("invalid hex character");
        return -1;
    }

#ifdef WITH_UNITTESTS
        static void unitTest ();
#endif

};

#endif /* PARSEHELPER_HPP */
