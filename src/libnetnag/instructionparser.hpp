/**
 * NETNAG <https://github.com/amartin755/netnag>
 * Copyright (C) 2012-2016 Andreas Martin (netnag@mailbox.org)
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


#ifndef INSTRUCTIONSPARSER_HPP_
#define INSTRUCTIONSPARSER_HPP_

#include <cstdint>
#include <cstdio>
#include <list>

#include "ethernetpacket.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"

class cParameterList;
class cIPv4Packet;

class cInstructionParser
{
public:
    cInstructionParser (const cMacAddress& ownMac, const cIpAddress& ownIPv4);
    ~cInstructionParser ();
    int parse (const char* instruction, uint64_t& timestamp, bool& isAbsolute, std::list <cEthernetPacket> &packets);

#ifdef WITH_UNITTESTS
        static void unitTest ();
#endif

private:
    int compileRAW (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileETH (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileARP (cParameterList& params, std::list <cEthernetPacket> &packets, bool isProbe = false, bool isGratuitous = false);
    int compileSNAP (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileIPv4 (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileUDP (cParameterList& params, std::list <cEthernetPacket> &packets);

    void compileMacHeader (cParameterList& params, cEthernetPacket& packet);
    int compileVLANTags (cParameterList& params, cEthernetPacket& packet);
    int compileIPv4Header (cParameterList& params, cIPv4Packet& packet);

    cMacAddress ownMac;
    cIpAddress  ownIPv4;
};

class ParseException
{
public:
    ParseException (const char* msg, const char* val)
    {
        this->msg = msg;
        this->val = val;
    }

    const char* what ()
    {
        return msg;
    }

    const char* value ()
    {
        return val;
    }

private:
    const char* msg;
    const char* val;
};


#endif /* INSTRUCTIONSPARSER_HPP_ */
