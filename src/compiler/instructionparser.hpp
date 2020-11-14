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


#ifndef INSTRUCTIONSPARSER_HPP_
#define INSTRUCTIONSPARSER_HPP_

#include <cstdint>
#include <cstddef>    // size_t
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
    class cResult
    {
    public:
        cResult (std::list <cEthernetPacket> &p) : packets (p)
        {
            clear ();
        }
        void clear (void)
        {
            timestamp = 0;
            isAbsolute = false;
            hasTimestamp = false;
        }

        bool hasTimestamp;
        uint64_t timestamp;
        bool isAbsolute;
        std::list <cEthernetPacket> &packets;
    };

    cInstructionParser (const cMacAddress& ownMac, const cIpAddress& ownIPv4);
    ~cInstructionParser ();
    int parse (const char* instruction, cResult& result);

#ifdef WITH_UNITTESTS
        static void unitTest ();
#endif

private:
    const char* parseTimestamp (const char* p, bool& hasTimestamp, uint64_t& timestamp, bool& isAbsolute);
    const char* parseProtocollIdentifier (const char* p, const char** identifier, size_t *len);

    int compileRAW  (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileETH  (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileARP  (cParameterList& params, std::list <cEthernetPacket> &packets, bool isProbe = false, bool isGratuitous = false);
    int compileSNAP (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileIPv4 (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileUDP  (cParameterList& params, std::list <cEthernetPacket> &packets);
    int compileVRRP (cParameterList& params, std::list <cEthernetPacket> &packets, int version);
    int compileSTP  (cParameterList& params, std::list <cEthernetPacket> &packets, bool isRSTP = false, bool isTCN = false);
    int compileIGMP (cParameterList& params, std::list <cEthernetPacket> &packets, bool v3, bool query, bool report, bool leave);

    bool   compileMacHeader (cParameterList& params, cEthernetPacket& packet, bool noDestination, bool destIsOptional = false);
    size_t compileVLANTags (cParameterList& params, cEthernetPacket& packet);
    bool   parseIPv4Params (cParameterList& params, cIPv4Packet& packet, bool noDestinationIP = false);

    void throwParseException (const char* msg, const char* val, size_t valLen = 0, const char* details = nullptr);

    cMacAddress ownMac;
    cIpAddress  ownIPv4;
    const char* currentInstruction;
};

class ParseException
{
public:
    ParseException (const char* inst, const char* errMsg, const char* errBegin, int errLen = 0)
    {
        this->errMsg  = errMsg;
        this->p    = errBegin;
        this->inst = inst;
        this->l    = errLen;
        this->d    = nullptr;
    }
    ParseException (const char* inst, const char* errMsg, const char* details, const char* errBegin, int errLen)
    {
        this->errMsg  = errMsg;
        this->p    = errBegin;
        this->inst = inst;
        this->l    = errLen;
        this->d    = details;
    }

    const char* errorMsg () const
    {
        return errMsg;
    }

    const char* instruction () const
    {
        return inst;
    }

    const char* errorBegin () const
    {
        return p;
    }

    int errorLen () const
    {
        return l;
    }

    const char* details () const
    {
        return d;
    }



private:
    const char* errMsg;
    const char* p;
    const char* inst;
    const char* d;
    int l;
};


#endif /* INSTRUCTIONSPARSER_HPP_ */
