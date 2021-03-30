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
#include <cstddef>
#include "cmdlineapp.hpp"
#include "ethernetpacket.hpp"
#include "pcapfileio.hpp"
#include "netinterface.hpp"

typedef struct
{
    const char*  ifc;
    int          repeat;
    int          verbosity;
    int          delay;
    const char*  timeRes;
    int          script;
    int          pcap;
    const char*  outpcap;
    const char*  myIP;
    const char*  myMAC;
    int          randSrcMac;
    int          randDstMac;
    int          arp;
    const char*  overwriteDMAC;
    int          testPredictableRandom;
    const char*  responderMode;
    const char*  bpf;
}appOptions;

class cInterface;
class cTimeval;
class cIpAddress;
class cMacAddress;
class ParseException;
class FileParseException;

class cTcpPump : public cCmdlineApp
{
public:
    cTcpPump (const char* name, const char* brief, const char* usage, const char* description);
    virtual ~cTcpPump();

    int execute (const std::list<std::string>& args);

private:
    void printParseError (const ParseException &e) const;
    void printFileParseError (const FileParseException &e) const;

    appOptions options;
    cPcapFileIO outfile;
    cTimeval activeDelay;
    unsigned timeScale; // 1 = us, 1000 = ms, 1000000 = sec
    bool realtimeMode;  // if true, packets will be sent time triggered

    enum reponder_mode {NONE = 0, ACK, MIRROR, TRIGGER};
    reponder_mode responder;
    cNetInterface* ifc;
};

#endif /* TCPPUMP_HPP */
