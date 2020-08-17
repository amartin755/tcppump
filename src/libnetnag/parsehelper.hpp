/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2019 Andreas Martin (netnag@mailbox.org)
 *
 * parsehelper.hpp
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

#ifndef PARSEHELPER_HPP
#define PARSEHELPER_HPP

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
    	// Values always start with an alphabetic character or a number
        return nextTokenStart (p, true, true, nullptr);
    }
    static const char* nextValueEnd (const char* p)
    {
    	// alphabetic characters, numbers, '.' and ':' are allowed
        return nextTokenEnd (p, true, true, ".:");
    }
    static const char* nextTokenStart (const char* p, bool alpha, bool num, const char* accept);
    static const char* nextTokenEnd (const char* p, bool alpha, bool num, const char* accept);
    static const char* nextCharIgnoreWhitspaces (const char* p, char c);
    static int isOneOf (char c, const char* accept);

#ifdef WITH_UNITTESTS
        static void unitTest ();
#endif

};

#endif /* PARSEHELPER_HPP */
