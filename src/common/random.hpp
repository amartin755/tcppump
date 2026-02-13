// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
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


#ifndef RANDOM_HPP_
#define RANDOM_HPP_

#include <cstdint>
#include <cfloat>
#include <limits>
#include <type_traits>

class cRandom
{
public:
    static cRandom* create (void);
    static void destroy (void);
    
    // Generic template for integral types
    template<typename T>
    static typename std::enable_if<std::is_integral<T>::value, T>::type
    rand (T min = 0, T max = std::numeric_limits<T>::max())
    {
        return (T)rand(static_cast<uint64_t>(min), static_cast<uint64_t>(max));
    }

    template<typename T>
    static typename std::enable_if<std::is_floating_point<T>::value, T>::type
    rand (T min = 0, T max = std::numeric_limits<T>::max())
    {
        return min + (max - min) * ((T)rand<uint64_t>() / (T)0xffffffffffffffff);
    }
    
    static void rand (void* p, size_t len);
    static void setCounterMode (unsigned startValue);

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    cRandom ();
    uint64_t pseudoRandom (void);
    uint64_t sequence (void);
    static uint64_t rand (uint64_t min = 0, uint64_t max = 0xffffffff);
    static cRandom* instance;
    bool countOnly;
    uint64_t seq;
};

#endif /* RANDOM_HPP_ */