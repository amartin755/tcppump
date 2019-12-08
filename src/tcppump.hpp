/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
 *
 * tcppump.hpp
 *
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
#include "libnetnag/protocoltypes.hpp"
#include "libnetnag/cmdlineapp.hpp"
#include "libnetnag/ethernetpacket.hpp"

typedef struct
{
    const char*  ifc;
    int          repeat;
    int          verbosity;
    int          delay;
    int          interactive;
    int          raw;
    int          script;
    int 		 pcap;
    const char*  inputmode; // raw, token, script, pcap
    const char*  keys;      // key bindings for interactive mode
}appOptions;

class cInterface;


class cTcpPump : public cCmdlineApp
{
public:
    cTcpPump (const char* name, const char* brief, const char* usage, const char* description);
    virtual ~cTcpPump();

    int execute (int argc, char* argv[]);

private:
    bool parsePackets (mac_t ownMac, ipv4_t ownIP, int cnt, char* packets[]);
    bool parseScripts (mac_t ownMac, ipv4_t ownIP , int cnt, char* scripts[]);
#if HAVE_PCAP
    bool parsePcapFiles (int cnt, char* pcaps[]);
#endif
    bool sendPacket (cInterface &ifc, unsigned delay, cEthernetPacket& p);
    bool interactiveMode (cInterface &ifc);

    appOptions options;
    std::list <cEthernetPacket> packets;
    std::map <int, cEthernetPacket&> keyBindings;
    unsigned triedToSendPackets;
    unsigned sentPackets;
    unsigned malformedPackets;
};

#endif /* TCPPUMP_HPP */
