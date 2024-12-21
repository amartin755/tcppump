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

uint32_t cRandom::rand32 (void)
{
    return rand ();
}

uint16_t cRandom::rand16 (void)
{
    return (uint16_t)rand ();
}

uint8_t cRandom::rand8 (void)
{
    return (uint8_t)rand ();
}

uint32_t cRandom::rand (void)
{
    assert (instance);
    return instance->countOnly ? instance->sequence() : instance->pseudoRandom();
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
                *(uint32_t*)p8 = instance->pseudoRandom ();
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

uint32_t cRandom::pseudoRandom (void)
{
    // TODO use lockless replacement for rand
    // see https://en.wikipedia.org/wiki/Xorshift#xoroshiro
    // and https://prng.di.unimi.it/xoroshiro128plus.c
    #if RAND_MAX < 65535
        return ((uint8_t)std::rand() << 24 | (uint8_t)std::rand() << 16 | ((uint8_t)std::rand() << 8) | (uint8_t)std::rand());
    #elif RAND_MAX < 4294967295
        return ((uint16_t)std::rand() << 16) | (uint16_t)std::rand();
    #else
        return (uint32_t)std::rand();
    #endif
}

uint32_t cRandom::sequence (void)
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
