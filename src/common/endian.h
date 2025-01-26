// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2025 Andreas Martin (netnag@mailbox.org)
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

#ifndef MY_ENDIAN_H_
#define MY_ENDIAN_H_

#if HAVE_BYTESWAP_H
# include <byteswap.h>
# if HAVE_BIG_ENDIAN
#  define toBE16(v) (v)
#  define toBE32(v) (v)
#  define toBE64(v) (v)
#  define toLE16(v) (bswap_16 (v))
#  define toLE32(v) (bswap_32 (v))
#  define toLE64(v) (bswap_64 (v))
# else
#  define toBE16(v) (bswap_16 (v))
#  define toBE32(v) (bswap_32 (v))
#  define toBE64(v) (bswap_64 (v))
#  define toLE16(v) (v)
#  define toLE32(v) (v)
#  define toLE64(v) (v)
# endif
#elif HAVE_WINDOWS
# if HAVE_BIG_ENDIAN
#  define toBE16(v) (v)
#  define toBE32(v) (v)
#  define toBE64(v) (v)
#  define toLE16(v) ((uint16_t)_byteswap_ushort(v))
#  define toLE32(v) ((uint32_t)_byteswap_ulong(v))
#  define toLE64(v) ((uint64_t)_byteswap_uint64(v))
# else
#  define toBE16(v) ((uint16_t)_byteswap_ushort(v))
#  define toBE32(v) ((uint32_t)_byteswap_ulong(v))
#  define toBE64(v) ((uint64_t)_byteswap_uint64(v))
#  define toLE16(v) (v)
#  define toLE32(v) (v)
#  define toLE64(v) (v)
# endif
#else
# error "no byteswap macros found"
#endif

#endif