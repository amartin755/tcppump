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


#ifndef FILEBACKEND_HPP_
#define FILEBACKEND_HPP_

#include <cstdint>

class cTimeval;
class cEthernetPacket;

class cFileBackend
{
public:
    virtual void write (const cTimeval& sendTime, cEthernetPacket& p) = 0;
    virtual void statistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const = 0;

    virtual ~cFileBackend() {};
};

#endif /* FILEBACKEND_HPP_ */
