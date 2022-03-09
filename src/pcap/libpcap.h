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
#ifndef LIBPCAP_H_
#define LIBPCAP_H_

#define HAVE_REMOTE 1
#include <pcap.h>
#if HAVE_WINDOWS
#include <Win32-Extensions.h>
#endif
/*
   #### workaround: pcap-stdinc.h does something like this:
    #  #if _MSC_VER < 1500
    #  #define snprintf _snprintf
    #  #define vsnprintf _vsnprintf
    #  #define strdup _strdup
    #  #endif
    # On non-ms compilers the if statement is always true and therefore can
    # lead to compile error with c++11 code
*/
#if _MSC_VER < 1500
#ifdef snprintf
#undef snprintf
#endif
#ifdef vsnprintf
#undef vsnprintf
#endif
#ifdef strdup
#undef strdup
#endif
#endif

#endif