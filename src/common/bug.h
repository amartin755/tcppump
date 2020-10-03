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


#ifndef BUGON_H
#define BUGON_H

#ifdef assert
#error do not combine assert.h and bug.h
#endif

#ifdef NDEBUG
#define NDEBUG_WAS_DEFINED 1
#endif

#undef NDEBUG
#include <assert.h>
#include <stdio.h>

//FIXME get rid of assert dependency

#define BUG_ON(expr) do{if (!(expr))fprintf (stderr, "Oops, you may found a bug!!!\n  "); assert ((expr));}while(0)
#define BUG(msg) BUG_ON(msg == 0)

#ifdef NDEBUG_WAS_DEFINED
#define NDEBUG 1
#endif

#undef NDEBUG_WAS_DEFINED

#endif /* BUGON_H */
