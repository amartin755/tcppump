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
    assert (instance);
    instance->countOnly = true;
    instance->seq       = startValue;
}

cRandom::cRandom () : countOnly(false), seq(0)
{
    if (!countOnly)
        std::srand ((unsigned)std::time (NULL));
}

int cRandom::rand (void)
{
    assert (instance);
    return instance->countOnly ? instance->sequence() : instance->pseudoRandom();
}

int cRandom::pseudoRandom (void)
{
    // TODO use lockless replacement for rand
    return std::rand();
}

int cRandom::sequence (void)
{
    if (seq > RAND_MAX)
        seq = 0;
    return (int)seq++;
}
