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


#ifndef TIMEVAL_H_
#define TIMEVAL_H_

/*
 * This header provides the definition of 'struct timeval'.
 * According to POSIX it is defined in sys/time.h, but not all windows compiler
 * provide this header. On the other hand this struct is part of winsock and thefore
 * a suitable fallback.
 *
 * Note: It's all about 'struct timeval'. All other symbols of sys/time.h
 *       are NOT the purpose of this header.
 */

#if HAVE_SYSTIME_H
# include <sys/time.h>
#elif HAVE_WS2
# include <winsock2.h>
#else
# error "no suitable header 'struct timeval' found"
#endif


#endif /* TIMEVAL_H_ */
