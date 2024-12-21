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


#ifndef INETCHECKSUM_HPP_
#define INETCHECKSUM_HPP_

#include <cstdint>
#include <cstddef>    // size_t

#include "bug.hpp"


class cInetChecksum
{
public:

    static uint16_t rfc1071 (const void* p, size_t len)
    {
        return rfc1071 (p, len, nullptr, 0, nullptr, 0);
    }
    static uint16_t rfc1071 (const void* p, size_t len, const void* p2, size_t len2)
    {
        BUG_ON (len & 1);
        return rfc1071 (p, len, p2, len2, nullptr, 0);
    }

    static uint16_t rfc1071 (const void* p, size_t len, const void* p2, size_t len2, const void* p3, size_t len3);


    static uint32_t rfc1071_calc (const void* p, size_t len, uint32_t sum = 0)
    {
        if (len == 0)
            return sum;

        if ((intptr_t)p & 1)
        {
            const uint8_t *p8 = (uint8_t*)p;

            while (len > 1)
            {
                uint16_t val;
#ifndef HAVE_BIG_ENDIAN
                val  = *p8++;
                val |= *p8++ << 8;
#else
                val  = *p8++ << 8;
                val |= *p8++;
#endif
                sum += val;
                len -= 2;
            }
            if (len)
                sum += *(uint8_t*)p8;
        }
        else
        {
            const uint16_t *p16 = (uint16_t*)p;

            while (len > 1)
            {
                sum += *p16++;
                len -= 2;
            }
            if (len)
#ifndef HAVE_BIG_ENDIAN
                sum += *(uint8_t*)p16;
#else
                sum += *(uint8_t*)p16 << 8;
#endif
        }
        return sum;
    }

private:
    static inline uint16_t rfc1071_finalize (uint32_t sum)
    {
        sum = (sum & 0xffff) + (sum >> 16);
        sum = (sum & 0xffff) + (sum >> 16);

        return (uint16_t)~sum;
    }

#ifdef WITH_UNITTESTS
public:
    static void unitTest ();
#endif
};



#endif /* INETCHECKSUM_HPP_ */
