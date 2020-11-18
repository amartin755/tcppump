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

#ifndef COMPILER_HPP_
#define COMPILER_HPP_

#include <string>
#include <list>
#include <iostream>
#include "packetdata.hpp"
#include "macaddress.hpp"
#include "ipaddress.hpp"
#include "timeval.hpp"


class cCompiler
{
public:

    enum inputType
    {
        PACKET, SCRIPT, PCAP
    };

    cCompiler (inputType type, const cMacAddress& ownMac, const cIpAddress& ownIP, const cTimeval& activeDelay, unsigned defaultDelayScale, bool ipOptionalDestMAC);
    cPacketData& operator<< (const std::list<std::string>& input);

private:
    void processPackets (const std::list<std::string>& input);
    void processScriptFiles (const std::list<std::string>& input);
#if HAVE_PCAP
    void processPcapFiles (const std::list<std::string>& input);
#endif

    cPacketData data;
    inputType type;
    const cMacAddress& ownMac;
    const cIpAddress& ownIP;
    const cTimeval& defaultDelay;
    unsigned defaultDelayScale;
    bool ipOptionalDestMAC;
};

#endif /* COMPILER_HPP_ */
