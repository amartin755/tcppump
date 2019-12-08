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
uint8_t* Converter::hexStringToBin (const char* hexString, int* binLength)
{
    int length = strlen (hexString);
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

bool Converter::hexStringToBin (const char* hexString, int hexStringLen, uint8_t* bin, int* binLength)
{
    assert (hexString);
    assert (binLength);

    int length = hexStringLen ? hexStringLen : strlen (hexString);

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

    for (int n = 0; n < length; n++)
    {
        if (!isxdigit (hexString[n]))
        {
            Console::PrintError ("Invalid character in hex string of frame payload\n");
            *binLength = 0;
            return false;
        }
    }

    for (int n = 0; n < length; n += 2)
    {
        char b[3] = {0};
        b[0] = hexString[n];
        b[1] = hexString[n + 1];
        bin[n/2] = (uint8_t)strtoul (b, NULL, 16);
    }
    *binLength = length / 2;

    return true;
}

// use optional parameter 'len' if 'str' is not null terminated or longer than a mac address
bool Converter::stringToMac (const char* str, mac_t& mac, int len)
{
    if (!checkMacString (str, len))
        return false;

    uint8_t* pMac = (uint8_t*)&mac;
    for (int n = 0; n < 17; n += 3)
    {
        *pMac++ = (uint8_t)strtoul (&str[n], NULL, 16);
    }

    return true;
}

bool Converter::checkMacString (const char* mac, size_t len)
{
    if (!len)
        len = strlen (mac);

    // 11:22:33:44:55:66

    if (len != 17)
        return false;

    for (size_t n = 0; n < len; n += 3)
    {
        if (!isxdigit (mac[n]) || !isxdigit (mac[n + 1]))
            return false;
        if ((n + 2) < len)
            if (mac[n + 2] != '-' && mac[n + 2] != ':')
                return false;
    }

    return true;
}


// use optional parameter 'len' if 'str' is not null terminated or longer than an ip address
bool Converter::stringToIpv4 (const char* str, ipv4_t& ip, int len)
{
    unsigned long ipElements[4] = {ULONG_MAX, ULONG_MAX, ULONG_MAX, ULONG_MAX};
    char substr[4];
    char* p = substr;
    int ipElementCnt = 0;
    unsigned substrLen = 0;
    int currLen = 0;

    if (!str)
        return false;

    if (!len)
        len = 15; // strlen("123.123.123.123") = 15

    do
    {
        if (isdigit (*str))
        {
            if (++substrLen > (sizeof (str) - 1))
                return false;

            *p++ = *str;
        }
        else if (*str == '.' || *str == '\0' || currLen >= len)
        {
            if (ipElementCnt >= 4 || !substrLen)
                return false;

            *p = '\0';
            p = substr;
            substrLen = 0;
            ipElements[ipElementCnt] = strtoul (p, NULL, 10);
            if (ipElements[ipElementCnt++] > 255)
                return false;
        }
        else
        {
            return false;
        }

    } while (*str++ != '\0' && currLen++ < len);

    if (ipElementCnt < 4)
        return false;

    ip = htonl ((ipElements[0] << 24) | (ipElements[1] << 16) | (ipElements[2] << 8) | ipElements [3]);
    return true;
}

#ifdef WITH_UNITTESTS
void Converter::unitTest ()
{
	nn::Console::PrintDebug("-- " __FILE__ " --\n");

    assert (!checkMacString(""));
    assert (!checkMacString("11:22:33:44:55:66:77"));
    assert (checkMacString("11:22:33:44:55:66"));
    assert (checkMacString("11-22-33-44-55-66"));
    assert (checkMacString("11:a2:33:44:55:66"));
    assert (!checkMacString("11:a2:3g:44:55:66"));
    assert (!checkMacString("11:a2:g3:44:55:66"));
    mac_t mac1 = {0};
    mac_t mac2 = {0xab,0x22,0x33,0x44,0x55,0x66};
    stringToMac ("ab:22:33:44:55:66", mac1);
    assert (!memcmp (&mac1, &mac2, sizeof (mac1)));

    int binLen;
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

    ipv4_t ip;
    assert (stringToIpv4 ("1.2.3.4", ip));
    assert (ip == inet_addr ("1.2.3.4"));
    assert (stringToIpv4 ("1.255.3.0", ip));
    assert (ip == inet_addr ("1.255.3.0"));
    assert (stringToIpv4 ("0.0.0.0", ip));
    assert (ip == inet_addr ("0.0.0.0"));
    assert (stringToIpv4 ("255.255.255.255", ip));
    assert (ip == inet_addr ("255.255.255.255"));

    assert (!stringToIpv4 ("1", ip));
    assert (!stringToIpv4 (".1", ip));
    assert (!stringToIpv4 ("1..2", ip));
    assert (!stringToIpv4 ("1.2", ip));
    assert (!stringToIpv4 ("1.2.", ip));
    assert (!stringToIpv4 ("1.2.3", ip));
    assert (!stringToIpv4 ("1.2.3.", ip));
    assert (!stringToIpv4 ("...", ip));
    assert (!stringToIpv4 (".1..", ip));
    assert (!stringToIpv4 ("4.256.3.6", ip));
    assert (!stringToIpv4 ("4.1111.3.6", ip));
    assert (!stringToIpv4 ("ab1.2.3.4", ip));
    assert (!stringToIpv4 ("1.2.3.4.5", ip));
    assert (!stringToIpv4 ("1.2.3.4.", ip));
    assert (!stringToIpv4 ("1.2.3.4ab", ip));
    assert (!stringToIpv4 ("1.2.ab3.4", ip));

}
#endif
}
