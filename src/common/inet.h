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


#ifndef INET_H_
#define INET_H_

/*
 * portable replacement for arpa/inet.h
 */

#if HAVE_ARPAINET_H
# include <arpa/inet.h> /* that's the posix way */
#elif HAVE_WS2
# include <winsock2.h> /* on windows those symbols are defined here */
# include <ws2tcpip.h>
#else
# error "no suitable header for htonl and friends found"
#endif


#endif /* INET_H_ */
