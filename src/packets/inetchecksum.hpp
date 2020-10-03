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

#ifndef INETCHECKSUM_HPP_
#define INETCHECKSUM_HPP_

#include <cstdint>
#include <cstddef>    // size_t

#include "../common/bug.h"

class cInetChecksum
{
public:

    static uint16_t rfc1071 (const void* p, size_t len)
    {
        return rfc1071 (p, len, nullptr, 0, nullptr, 0);
    }
    static uint16_t rfc1071 (const void* p, size_t len, const void* p2, size_t len2)
    {
        return rfc1071 (p, len, p2, len2, nullptr, 0);
    }
    static inline uint16_t rfc1071 (const void* p, size_t len, const void* p2, size_t len2, const void* p3, size_t len3)
    {
        uint32_t sum = 0;


        if (p)
            sum  = rfc1071_calc ((uint16_t*)p, len);
        if (p2)
            sum += rfc1071_calc ((uint16_t*)p2, len2);
        if (p3)
            sum += rfc1071_calc ((uint16_t*)p3, len3);

        return rfc1071_finalize (sum);
    }

    static uint32_t rfc1071_calc (const uint16_t* p, size_t len)
    {
        uint32_t sum = 0;

        while (len > 1)
        {
            sum += *p++;
            len -= 2;
        }
        if (len)
            sum += *(uint8_t*)p;

        return sum;
    }

private:
    static inline uint16_t rfc1071_finalize (uint32_t sum)
    {
        uint16_t chksum;
        while (sum >> 16)
            sum = (sum & 0xffff) + (sum >> 16);

        chksum = (uint16_t)~sum;
        return chksum;
    }

#ifdef WITH_UNITTESTS
public:
    static void unitTest ()
    {
        uint8_t hd[] = {0x45,0x00,0x02,0x03,0x16,0xd1,0x00,0x00,0x01,0x11,0,0,0xc0,0xa8,0x00,0x88,0xef,0xff,0xff,0xfa};
        BUG_ON (cInetChecksum::rfc1071 ((uint16_t*)hd, sizeof (hd), nullptr, 0) == 0xeeef);
        uint8_t hd1[] = {0x45,0x00,0x02,0x03,0x16,0xd1,0x00,0x00,0x01,0x11,0xef,0xee,0xc0,0xa8,0x00,0x88,0xef,0xff,0xff,0xfa};
        BUG_ON (cInetChecksum::rfc1071 ((uint16_t*)hd1, sizeof (hd1), nullptr, 0) == 0);
    }
#endif
};



#endif /* INETCHECKSUM_HPP_ */
