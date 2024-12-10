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


#ifndef BUGON_HPP
#define BUGON_HPP

#include <cstdio>
#include <cstdlib>
#include <cassert>

#ifdef __GNUC__
#    define unlikely(x)     __builtin_expect((x),0)
#else
#    define unlikely(x)     (x)
#endif


static inline void __game_over (const char* expr, const char* file, int line)
{
    std::fprintf (stderr, "Oops, you may found a bug!!!\n %s %d: '%s'\n", file, line, expr);
#ifndef NDEBUG // this ensures triggering SEGFAULT in ctest
    *((volatile char*)0) = 1;
#else
    std::abort ();
#endif
}
#define BUG_ON(expr)                             \
     (unlikely(static_cast <bool> (expr))        \
      ? __game_over (#expr, __FILE__, __LINE__)  \
      : void (0))

#define BUG(msg) __game_over (#msg, __FILE__, __LINE__)
#define BUG_IF_NOT(expr) BUG_ON (!(expr))

#endif /* BUGON_HPP */
