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


#ifndef COMPILER_HPP_
#define COMPILER_HPP_

#include <string>
#include <vector>
#include <iostream>
#include "packetdata.hpp"
#include "macaddress.hpp"
#include "ipaddress.hpp"
#include "timeval.hpp"
#include "fileparser.hpp"


class cCompiler
{
public:

    enum inputType
    {
        PACKET, SCRIPT, PCAP
    };

    cCompiler (inputType type, const cTimeval& activeDelay, unsigned defaultDelayScale, bool ipOptionalDestMAC, double pcapScaling);
    cPacketData& operator<< (const std::vector<std::string>& input);

private:
    void processPackets (const std::vector<std::string>& input);
    void processScriptFiles (const std::vector<std::string>& input);
    void processPcapFiles (const std::vector<std::string>& input);

    cPacketData data;
    inputType type;
    const cTimeval& defaultDelay;
    unsigned defaultDelayScale;
    bool ipOptionalDestMAC;
    cFileParser fileParser;
    double pcapScalingFactor;
};

#endif /* COMPILER_HPP_ */
