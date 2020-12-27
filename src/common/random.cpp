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

#include <cstdlib>
#include <ctime>
#include <cassert>
#include "random.hpp"

cRandom* cRandom::instance = nullptr;

cRandom* cRandom::create (bool countOnly)
{
    if (!instance)
    {
        instance = new cRandom(countOnly);
    }
    return instance;
}

cRandom::cRandom (bool countOnly)
{
    this->countOnly = countOnly;

    if (!countOnly)
        std::srand ((unsigned)std::time (NULL));
    else
        seq = 0;
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
