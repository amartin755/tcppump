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

#ifndef PACKETDATA_HPP_
#define PACKETDATA_HPP_

#include <list>
#include "ethernetpacket.hpp"
#include "timeval.hpp"

class cPacketData
{
public:
    cPacketData ()
    {
        hasUserTimestamps = false;
    }
    std::list <cEthernetPacket> packets;
    std::list <cTimeval> timestamps;
    bool hasUserTimestamps; // is true, if at least one timestamp in timestamp list is user defined (no default value)
};

#endif /* PACKETDATA_HPP_ */
