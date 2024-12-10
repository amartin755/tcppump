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
    BUG_ON (!instance);
    return instance->countOnly ? instance->sequence() : instance->pseudoRandom();
}

uint32_t cRandom::pseudoRandom (void)
{
    // TODO use lockless replacement for rand
    return ((uint32_t)std::rand() << 30) | ((uint32_t)std::rand() << 15) | (uint32_t)std::rand();
}

uint32_t cRandom::sequence (void)
{
    return seq++;
}
