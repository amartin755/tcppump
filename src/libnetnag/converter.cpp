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


#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <climits>

#include "console.hpp"
#include "inet.h"
#include "converter.hpp"

namespace nn
{

// the function will alloc a buffer. The caller has to free this buffer!
uint8_t* Converter::hexStringToBin (const char* hexString, size_t* binLength)
{
    size_t length = strlen (hexString);
    uint8_t* binary = (uint8_t*)malloc (length / 2);


    if (!binary)
    {
        *binLength = 0;
        Console::PrintError ("Not enough memory\n");
    }
    else
    {
        *binLength = length / 2;
        if (!hexStringToBin (hexString, length, binary, binLength))
        {
            free (binary);
            binary = NULL;
        }
    }
    return binary;
}

bool Converter::hexStringToBin (const char* hexString, size_t hexStringLen, uint8_t* bin, size_t* binLength)
{
    assert (hexString);
    assert (binLength);

    size_t length = hexStringLen ? hexStringLen : strlen (hexString);

    if (!length)
    {
        Console::PrintError ("Zero length of hex string of frame payload\n");
        *binLength = 0;
        return false;
    }
    if (length & 1)
    {
        Console::PrintError ("Uneven length of hex string of frame payload\n");
        *binLength = 0;
        return false;
    }

    assert ((length / 2) <= *binLength);

    for (size_t n = 0; n < length; n++)
    {
        if (!isxdigit (hexString[n]))
        {
            Console::PrintError ("Invalid character in hex string of frame payload\n");
            *binLength = 0;
            return false;
        }
    }

    for (size_t n = 0; n < length; n += 2)
    {
        char b[3] = {0};
        b[0] = hexString[n];
        b[1] = hexString[n + 1];
        bin[n/2] = (uint8_t)strtoul (b, NULL, 16);
    }
    *binLength = length / 2;

    return true;
}

#ifdef WITH_UNITTESTS
void Converter::unitTest ()
{
    nn::Console::PrintDebug("-- " __FILE__ " --\n");

    size_t binLen;
    uint8_t* bin;
    binLen = -1;
    assert (!hexStringToBin ("", &binLen));
    assert (!binLen);
    binLen = -1;
    assert (!hexStringToBin ("1abcdef", &binLen));
    assert (!binLen);
    binLen = -1;
    assert (!hexStringToBin ("abcdefg", &binLen));
    assert (!binLen);
    binLen = -1;
    assert (!hexStringToBin ("1abcdefg", &binLen));
    assert (!binLen);
    binLen = -1;
    assert ((bin = hexStringToBin ("0123456789abcdef", &binLen)));
    assert (binLen == 8);
    assert (!memcmp (bin, "\x01\x23\x45\x67\x89\xab\xcd\xef", binLen));
    binLen = -1;
    free ((void*)bin);

    uint8_t binbuf[16] = {0};
    binLen = sizeof (binbuf);
    assert (hexStringToBin ("0123456789abcdef", 0, binbuf, &binLen));
    assert (binLen == 8);
    assert (!memcmp (binbuf, "\x01\x23\x45\x67\x89\xab\xcd\xef\00", binLen+1));
    binLen = sizeof (binbuf);
    binbuf [2] = 0;
    assert (hexStringToBin ("0123456789abcdef", 4, binbuf, &binLen));
    assert (binLen == 2);
    assert (!memcmp (binbuf, "\x01\x23\x00\x67\x89\xab\xcd\xef\00", 9));
    binLen = 4;
}
#endif
}
