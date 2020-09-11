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


#ifndef TCPPUMP_HPP
#define TCPPUMP_HPP


#include <list>
#include <map>
#include <cstddef>
#include "cmdlineapp.hpp"
#include "ethernetpacket.hpp"
#include "pcapfileio.hpp"

typedef struct
{
    const char*  ifc;
    int          repeat;
    int          verbosity;
    int          delay;
    const char*  timeRes;
    int          interactive;
    int          raw;
    int          script;
    int          pcap;
    const char*  inputmode; // raw, token, script, pcap
    const char*  keys;      // key bindings for interactive mode
    const char*  outpcap;
    const char*  myIP;
    const char*  myMAC;
    int          dissect;
}appOptions;

class cInterface;
class cTimeval;
class cIpAddress;
class cMacAddress;

class cTcpPump : public cCmdlineApp
{
public:
    cTcpPump (const char* name, const char* brief, const char* usage, const char* description);
    virtual ~cTcpPump();

    int execute (int argc, char* argv[]);

private:
    bool parsePackets (const cMacAddress& ownMac, const cIpAddress& ownIP, int cnt, char* packets[]);
    bool parseScripts (const cMacAddress& ownMac, const cIpAddress& ownIP , int cnt, char* scripts[]);
#if HAVE_PCAP
    bool parsePcapFiles (int cnt, char* pcaps[]);
#endif
    bool sendPacket (cInterface &ifc, const cTimeval &delay, const cEthernetPacket &p);
    bool interactiveMode (cInterface &ifc);

    appOptions options;
    std::list <cEthernetPacket> packets;
    std::list <cTimeval> delays;
    std::map <int, cEthernetPacket&> keyBindings;
    unsigned triedToSendPackets;
    unsigned sentPackets;
    unsigned malformedPackets;
#if HAVE_PCAP
    cPcapFileIO outfile;
#endif
    cTimeval activeDelay;
    unsigned timeScale; // 1 = us, 1000 = ms, 1000000 = sec
};

#endif /* TCPPUMP_HPP */
