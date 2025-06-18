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


#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#include "parsehelper.hpp"
#include "bug.hpp"


const char* cParseHelper::skipWhitespaces (const char* p)
{
    while (isspace (*p))
        p++;

    return p;
}

const char* cParseHelper::nextTokenStart (const char* p, bool alpha, bool num, const char* accept)
{
    // ignore whitespaces
    p = skipWhitespaces(p);

    if ((alpha && isalpha (*p)) || (num && isdigit (*p)) || (accept && isOneOf (*p, accept)))
        return p;

    return nullptr;
}

const char* cParseHelper::nextTokenEnd (const char* p, bool alpha, bool num, const char* accept)
{
    while (*p != '\0')
    {
        if ((alpha && isalpha (*p)) || (num && isdigit (*p)) || (accept && isOneOf (*p, accept)))
            p++;
        else
            return p;
    }
    return p;
}

const char* cParseHelper::nextCharIgnoreWhitspaces (const char* p, char c)
{
    p = skipWhitespaces(p);
    return *p == c ? p : nullptr;
}

int cParseHelper::isOneOf (char c, const char* accept)
{
    BUG_ON (!accept);

    while (*accept != '\0')
        if (*accept++ == c)
            return 1;

    return 0;
}

// the function will alloc a buffer. The caller has to free this buffer!
uint8_t* cParseHelper::hexStringToBin (const char* hexString, size_t hexStringLen, size_t& binLength)
{
    BUG_ON (!hexString);

    binLength = 0;

    size_t length = hexStringLen ? hexStringLen : std::strlen (hexString);

    if (!length)
    {
//        Console::PrintError ("Zero length of hex string of frame payload\n");
        return nullptr;
    }
    if (length & 1)
    {
//        Console::PrintError ("Uneven length of hex string of frame payload\n");
        return nullptr;
    }

    for (size_t n = 0; n < length; n++)
    {
        if (!isxdigit (hexString[n]))
        {
//            Console::PrintError ("Invalid character in hex string of frame payload\n");
            return nullptr;
        }
    }

    uint8_t* bin = new uint8_t[length / 2];


    for (size_t n = 0; n < length; n += 2)
    {
        char b[3] = {0};
        b[0] = hexString[n];
        b[1] = hexString[n + 1];
        bin[n/2] = (uint8_t)std::strtoul (b, NULL, 16);
    }
    binLength = length / 2;

    return bin;
}

bool cParseHelper::range (const char* p, size_t len, int base, uint64_t& begin, uint64_t& end)
{
    unsigned long long b, e;
    char* numend;

    BUG_ON (!p);

    //smallest valid range "(n-n)"
    if (len < 5)
        return false;

    if (*p != '(' || *(p + len - 1) != ')')
        return false;

    if (isspace (*(p + 1)))
        return false;
    b = strtoull (p + 1, &numend, base);
    if (errno == ERANGE || *numend != '-' || (numend + 1) >= (p + len))
        return false;
    if (isspace (*(numend + 1)))
        return false;
    e = strtoull (numend + 1, &numend, base);
    if (errno == ERANGE || *numend != ')' || numend >= (p + len))
        return false;

    begin = b;
    end = e;
    return true;
}


#ifdef WITH_UNITTESTS

#include "console.hpp"

void cParseHelper::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    size_t binLen;
    uint8_t* bin;
    binLen = -1;
    BUG_IF_NOT (!hexStringToBin ("", 0, binLen));
    BUG_IF_NOT (!binLen);
    binLen = -1;
    BUG_IF_NOT (!hexStringToBin ("1abcdef", 0, binLen));
    BUG_IF_NOT (!binLen);
    binLen = -1;
    BUG_IF_NOT (!hexStringToBin ("abcdefg", 0, binLen));
    BUG_IF_NOT (!binLen);
    binLen = -1;
    BUG_IF_NOT (!hexStringToBin ("1abcdefg", 0, binLen));
    BUG_IF_NOT (!binLen);
    binLen = -1;
    BUG_IF_NOT ((bin = hexStringToBin ("0123456789abcdef", 0, binLen)));
    BUG_IF_NOT (binLen == 8);
    BUG_IF_NOT (!memcmp (bin, "\x01\x23\x45\x67\x89\xab\xcd\xef", binLen));
    binLen = -1;
    delete[] bin;


    BUG_IF_NOT (isOneOf ('a', "abcdef"));
    BUG_IF_NOT (isOneOf ('c', "abcdef"));
    BUG_IF_NOT (isOneOf ('f', "abcdef"));
    BUG_IF_NOT (!isOneOf ('r', "abcdef"));
    BUG_IF_NOT (!isOneOf ('r', ""));

    BUG_IF_NOT (*skipWhitespaces ("abc") == 'a');
    BUG_IF_NOT (*skipWhitespaces (" abc") == 'a');
    BUG_IF_NOT (*skipWhitespaces (" \t\nabc") == 'a');
    BUG_IF_NOT (*skipWhitespaces ("") == '\0');

    BUG_IF_NOT (!nextCharIgnoreWhitspaces (" abcd\tef\ng", 'f'));
    BUG_IF_NOT (!nextCharIgnoreWhitspaces (" abcd\tef\ng", 'b'));
    BUG_IF_NOT (*nextCharIgnoreWhitspaces (" abcd\tef\ng", 'a') == 'a');

    {
        const char s[] = "abc de-0f  ghi";
        const char* p = nullptr;
        p = nextKeyStart (s);
        BUG_IF_NOT (p == &s[0]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[3]);
        p = nextKeyStart (p);
        BUG_IF_NOT (p == &s[4]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[9]);
        p = nextKeyStart (p);
        BUG_IF_NOT (p == &s[11]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[14]);
    }
    {
        const char s[] = "  abc \tde-0f  ghi";
        const char* p = nullptr;
        p = nextKeyStart (s);
        BUG_IF_NOT (p == &s[2]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[5]);
        p = nextKeyStart (p);
        BUG_IF_NOT (p == &s[7]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[12]);
        p = nextKeyStart (p);
        BUG_IF_NOT (p == &s[14]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[17]);
    }
    {
        const char s[] = "a b c ";
        const char* p = nullptr;
        p = nextKeyStart (s);
        BUG_IF_NOT (p == &s[0]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[1]);
        p = nextKeyStart (p);
        BUG_IF_NOT (p == &s[2]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[3]);
        p = nextKeyStart (p);
        BUG_IF_NOT (p == &s[4]);
        p = nextKeyEnd (p);
        BUG_IF_NOT (p == &s[5]);
    }
    {
        const char s[] = "-a";
        const char* p;
        p = nextKeyStart (s);
        BUG_IF_NOT (!p);
    }
    {
        const char s[] = "5a";
        const char* p;
        p = nextKeyStart (s);
        BUG_IF_NOT (!p);
    }

    struct testcase_t
    {
        const char* range;
        int base;
        uint64_t begin;
        uint64_t end;
        bool ret;
    };

    const testcase_t ranges[] = {
        {
            "(1-2)",
            0,
            1,
            2,
            true
        },
        {
            "(1-2)",
            10,
            1,
            2,
            true
        },
        {
            "(1b-cd)",
            16,
            0x1b,
            0xcd,
            true
        },
        {
            "(0x1b-0xcd)",
            0,
            0x1b,
            0xcd,
            true
        },
        {
            "(0x1b-100)",
            0,
            0x1b,
            100,
            true
        },
        {
            "(1-2",
            0,
            1,
            2,
            false
        },
        {
            "1-2)",
            0,
            1,
            2,
            false
        },
        {
            "(1 -2)",
            0,
            1,
            2,
            false
        },
        {
            "(1- 2)",
            0,
            1,
            2,
            false
        },
        {
            "(1 - 2)",
            0,
            1,
            2,
            false
        },
        { // 10
            "( 1-2)",
            0,
            1,
            2,
            false
        },
        {
            "(1-2 )",
            0,
            1,
            2,
            false
        },
        {
            " (1-2)",
            0,
            1,
            2,
            false
        },
        {
            "(1-2) ",
            0,
            1,
            2,
            false
        },
        {
            "(1-)",
            0,
            1,
            2,
            false
        },
        {
            "(1- ) ",
            0,
            1,
            2,
            false
        },
        {
            "( -2)",
            0,
            1,
            2,
            false
        },
        { // 17
            "(-2)",
            0,
            1,
            2,
            false
        },
        {
            "(-)",
            0,
            1,
            2,
            false
        },
        {
            "()",
            0,
            1,
            2,
            false
        },
        { // 20
            "(",
            0,
            1,
            2,
            false
        },
        {
            ")",
            0,
            1,
            2,
            false
        },
        {
            "",
            0,
            1,
            2,
            false
        },
        {
            " ",
            0,
            1,
            2,
            false
        },
        {
            "(1--2)",
            0,
            1,
            uint64_t(-2),
            true
        },
        {
            "(-1-2)",
            0,
            uint64_t(-1),
            2,
            true
        },
    };

    for (unsigned n = 0; n < sizeof(ranges)/sizeof(ranges[0]); n++)
    {
        uint64_t begin = 0;
        uint64_t end = 0;
        Console::PrintDebug("range %d: ", n);
        BUG_ON (range (ranges[n].range, std::strlen (ranges[n].range), ranges[n].base, begin, end) != ranges[n].ret);
        if (ranges[n].ret)
            BUG_ON (begin != ranges[n].begin || end != ranges[n].end);
        else
            BUG_ON (begin != 0 || end != 0);
        Console::PrintDebug("\r");
    }

    // TODO much more detailed tests
}
#endif
