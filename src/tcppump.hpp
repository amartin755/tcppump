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


#ifndef TCPPUMP_HPP
#define TCPPUMP_HPP


#include <cstddef>
#include "cmdlineapp.hpp"
#include "ethernetpacket.hpp"
#include "pcapfileio.hpp"
#include "netinterface.hpp"

struct appOptions
{
    const char*  ifc;
    int          repeat;
    int          delay;
    const char*  timeRes;
    int          script;
    int          pcap;
    const char*  pcapScaling;
    const char*  outfile;
    const char*  myIP;
    const char*  myIPv6;
    const char*  myMAC;
    int          randSrcMac;
    int          randDstMac;
    int          arp;
    const char*  overwriteDMAC;
    int          testPredictableRandom;
    unsigned     mtu;
    const char*  outFormat;
};

class cInterface;
class cTimeval;
class cIPv4;
class cMacAddress;
class ParseException;
class FileParseException;

class cTcpPump : public cCmdlineApp
{
public:
    cTcpPump (const char* name, const char* brief, const char* usage, const char* description,
        const char* version, const char* build, const char* buildDetails);
    virtual ~cTcpPump();

    int execute (const std::vector<std::string>& args);

private:
    void printParseError (const ParseException &e) const;
    void printFileParseError (const FileParseException &e) const;

    appOptions options;
    cPcapFileIO outfile;
    cTimeval activeDelay;
    unsigned timeScale; // 1 = us, 1000 = ms, 1000000 = sec
    bool realtimeMode;  // if true, packets will be sent time triggered

    cNetInterface* ifc;
};

#endif /* TCPPUMP_HPP */
