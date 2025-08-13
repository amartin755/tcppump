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


#include <cstdlib>
#include <ctime>
#include <cassert>
#include <cstring>
#include <random>

#include "bug.hpp"
#include "random.hpp"

cRandom* cRandom::instance = nullptr;

cRandom* cRandom::create (void)
{
    if (!instance)
    {
        instance = new cRandom();
    }
    return instance;
}

void cRandom::destroy (void)
{
    if (instance)
    {
        delete instance;
        instance = nullptr;
    }
}

void cRandom::setCounterMode (unsigned startValue)
{
    BUG_ON (!instance);
    instance->countOnly = true;
    instance->seq       = startValue;
}

cRandom::cRandom () : countOnly(false), seq(0)
{
    if (!countOnly)
        std::srand ((unsigned)std::time (NULL));
}

uint64_t cRandom::rand (uint64_t min, uint64_t max)
{
    assert (instance);
    uint64_t r = instance->countOnly ? instance->sequence() : instance->pseudoRandom();

    uint64_t range = max - min + 1;

    // if min...max spans the entire 64bit area, range becomes zero
    // In this case we need special handling to avoid division by zero.
    return (range ? r % range : r) + min;
}

void cRandom::rand (void* p, size_t len /*number of bytes*/)
{
    assert (instance);

    uint8_t *data = (uint8_t*)p;

    if (instance->countOnly)
    {
        for (size_t n = 0; n < len; n++)
        {
            data[n] = (uint8_t)n;
        }
    }
    else
    {
        uint8_t* p8 = (uint8_t*)p;

        if ((intptr_t)p8 & 1)
        {
            *p8++ = (uint8_t)instance->pseudoRandom ();
            len--;
        }
        if (len >= 2)
        {
            if ((intptr_t)p8 & 2)
            {
                *(uint16_t*)p8 = (uint16_t)instance->pseudoRandom ();
                p8 += 2;
                len -= 2;
            }
            while (len >= 4)
            {
                *(uint32_t*)p8 = (uint32_t)instance->pseudoRandom ();
                p8 += 4;
                len -= 4;
            }
            if (len & 2)
            {
                *(uint16_t*)p8 = (uint16_t)instance->pseudoRandom ();
                p8 += 2;
                len -= 2;
            }
        }
        if (len & 1)
        {
            *p8++ = (uint8_t)instance->pseudoRandom ();
            len--;
        }
    }
}

uint64_t cRandom::pseudoRandom (void)
{
    static thread_local std::mt19937_64 rng(std::random_device{}());
    return rng();
}

uint64_t cRandom::sequence (void)
{
    return seq++;
}

#ifdef WITH_UNITTESTS
#include "console.hpp"
void cRandom::unitTest ()
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    uint32_t data[32];
    uint8_t* a = (uint8_t*)&data[1];
    for (int n = 0; n < 3; n++, a++)
    {
        {
            std::memset (data, 0, sizeof (data));
            rand (a, 1);
            BUG_IF_NOT (data[0] == 0);
            BUG_IF_NOT (a[1] == 0);
        }
        {
            std::memset (data, 0, sizeof (data));
            rand (a, 2);
            BUG_IF_NOT (data[0] == 0);
            BUG_IF_NOT (a[2] == 0);
        }
        {
            std::memset (data, 0, sizeof (data));
            rand (a, 3);
            BUG_IF_NOT (data[0] == 0);
            BUG_IF_NOT (a[3] == 0);
        }
        {
            std::memset (data, 0, sizeof (data));
            rand (a, 4);
            BUG_IF_NOT (data[0] == 0);
            BUG_IF_NOT (a[4] == 0);
        }
        {
            std::memset (data, 0, sizeof (data));
            rand (a, 5);
            BUG_IF_NOT (data[0] == 0);
            BUG_IF_NOT (a[5] == 0);
        }
        {
            std::memset (data, 0, sizeof (data));
            rand (a, 6);
            BUG_IF_NOT (data[0] == 0);
            BUG_IF_NOT (a[6] == 0);
        }
        {
            std::memset (data, 0, sizeof (data));
            rand (a, 10);
            BUG_IF_NOT (data[0] == 0);
            BUG_IF_NOT (a[10] == 0);
        }
    }
}
#endif
