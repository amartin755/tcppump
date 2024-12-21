// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2024 Andreas Martin (netnag@mailbox.org)
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

#include <cstring>

#include "bug.hpp"
#include "inetchecksum.hpp"

// TODO
// maybe it's woth to have a closer look at the kernels implementation
// https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/lib/checksum.c

uint16_t cInetChecksum::rfc1071 (const void* p, size_t len, const void* p2, size_t len2, const void* p3, size_t len3)
{
    // p, p2, p3 are allowed to be misalligend
    // only the lenght of the LAST used parameter pair is allowed to be odd!
    BUG_ON (len3 && (len & 1) && (len2 & 1));
    uint32_t sum = 0;

    if (p)
    {
        sum  = rfc1071_calc (p, len, sum);
    }
    if (p2)
    {
        sum = rfc1071_calc (p2, len2, sum);
    }
    if (p3)
    {
        sum = rfc1071_calc (p3, len3, sum);
    }

    return rfc1071_finalize (sum);
}


#ifdef WITH_UNITTESTS
#include "console.hpp"
void cInetChecksum::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    uint8_t hd[] = {0x45,0x00,0x02,0x03,0x16,0xd1,0x00,0x00,0x01,0x11,0,0,0xc0,0xa8,0x00,0x88,0xef,0xff,0xff,0xfa};
    BUG_IF_NOT (cInetChecksum::rfc1071 (hd, sizeof (hd)) == 0xeeef);
    uint8_t hd1[] = {0x45,0x00,0x02,0x03,0x16,0xd1,0x00,0x00,0x01,0x11,0xef,0xee,0xc0,0xa8,0x00,0x88,0xef,0xff,0xff,0xfa};
    BUG_IF_NOT (cInetChecksum::rfc1071 (hd1, sizeof (hd1)) == 0);
    // partial calculation must return the same result
    BUG_IF_NOT (cInetChecksum::rfc1071 (hd1, 4, &hd1[4], 8, &hd1[12], 8) == 0);
    // 16bit aligned data must return the same result
    uint16_t hd16[sizeof (hd1)/2];
    static_assert (sizeof (hd16) == sizeof (hd1), "");
    BUG_ON ((intptr_t)hd16 & 1);
    std::memcpy (hd16, hd1, sizeof (hd16));
    BUG_IF_NOT (cInetChecksum::rfc1071 (hd16, sizeof (hd16)) == 0);
    // 32bit aligned data must return the same result
    uint32_t hd32[sizeof (hd16)/4];
    static_assert (sizeof (hd16) == sizeof (hd32), "");
    BUG_ON ((intptr_t)hd32 & 3);
    std::memcpy (hd32, hd16, sizeof (hd32));
    BUG_IF_NOT (cInetChecksum::rfc1071 (hd32, sizeof (hd32)) == 0);

    uint32_t unaligned[sizeof (hd1)];
    uint8_t *p8 = ((uint8_t*)unaligned)+1;
    std::memset (unaligned, 0xFE, sizeof (unaligned));
    std::memcpy (p8, hd1, sizeof (hd1));
    BUG_IF_NOT (cInetChecksum::rfc1071 (p8, sizeof (hd1)) == 0);
    {
        const uint8_t ipheader[] = {1,2,3,4, 10,20,30,40, 0, 17, 0, 24};
        const uint8_t udpheadr[] = {0,1 ,0,2, 0,24, 0,0};
        const uint8_t payload[]  = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
        BUG_IF_NOT (cInetChecksum::rfc1071 (ipheader, sizeof (ipheader), udpheadr, sizeof (udpheadr), payload, sizeof(payload)) == 0x2e97);
    }
    // do something with a odd payload
    {
        const uint8_t ipheader[] = {1,2,3,4, 0xe0,0x14,0x1e,0x28, 0, 17, 0, 39};
        const uint8_t udpheadr[] = {0,0 ,0,0, 0,0x27, 0,0};
        const char* payload = "There's no place like 127.0.0.1";
        BUG_IF_NOT (cInetChecksum::rfc1071 (ipheader, sizeof (ipheader), udpheadr, sizeof (udpheadr), payload, std::strlen(payload)) == 0xe023);
    }
    // do the same but force the payload to be misaligned
    {
        const uint8_t ipheader[] = {1,2,3,4, 0xe0,0x14,0x1e,0x28, 0, 17, 0, 39};
        const uint8_t udpheadr[] = {0,0 ,0,0, 0,0x27, 0,0};
        const char* payload = " There's no place like 127.0.0.1";
        BUG_IF_NOT (cInetChecksum::rfc1071 (ipheader, sizeof (ipheader), udpheadr, sizeof (udpheadr), payload+1, std::strlen(payload+1)) == 0xe023);
    }
}
#endif
