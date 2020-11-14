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


#ifndef BUGON_HPP
#define BUGON_HPP

#include <cstdio>
#include <cstdlib>

static inline void __game_over (const char* expr, const char* file, int line)
{
    std::fprintf (stderr, "Oops, you may found a bug!!!\n %s %d: '%s'\n", file, line, expr);
    std::abort ();
}
#define BUG_ON(expr)                            \
     (static_cast <bool> (expr)                        \
      ? void (0)                            \
      : __game_over (#expr, __FILE__, __LINE__))
#define BUG(msg) __game_over (#msg, __FILE__, __LINE__)

#endif /* BUGON_HPP */