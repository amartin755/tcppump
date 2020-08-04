/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * protocoltypes.hpp
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

// FIXME get rid of this code! ipv4_t and mac_t must die

#ifndef PROTOCOLTYPES_HPP_
#define PROTOCOLTYPES_HPP_

#include <cstdint>
#include <cstdlib>
#include <ctime>

//typedef uint32_t ipv4_t;

#pragma pack(1)

typedef struct
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t f;

    bool isNull () const
    {
        return !a && !b && !c && !d && !e && !f;
    }
    bool isBroadcast () const
    {
        return a == 0xffu && b == 0xffu && c == 0xffu && d == 0xffu && e == 0xffu && f == 0xffu;
    }
    bool isMulticast () const
    {
        return (a & 1) && !isBroadcast ();
    }
    void set (uint8_t val)
    {
        a = b = c = d = e = f = val;
    }
    void setRandom ()
    {
        srand (time (NULL));
        a = uint8_t(rand () % 256) & 0xfc; // unicast, global unique
        b = uint8_t(rand () % 256);
        c = uint8_t(rand () % 256);
        d = uint8_t(rand () % 256);
        e = uint8_t(rand () % 256);
        f = uint8_t(rand () % 256);
    }
}mac_t;


typedef struct
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
}oui_t;

#pragma pack()


#endif /* PROTOCOLTYPES_HPP_ */
