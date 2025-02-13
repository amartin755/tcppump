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


#ifndef INSTRUCTIONSPARSER_HPP_
#define INSTRUCTIONSPARSER_HPP_

#include <cstdint>
#include <cstddef>    // size_t
#include <cstdio>
#include <list>

#include "ethernetpacket.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "linkable.hpp"

class cParameterList;
class cParameter;
class cIPPacket;


class cInstructionParser
{
public:
    class cResult
    {
    public:
        cResult ()
        {
            clear ();
        }
        void clear (void)
        {
            timestamp = 0;
            isAbsolute = false;
            hasTimestamp = false;
            packets = nullptr;
        }

        bool hasTimestamp;
        uint64_t timestamp;
        bool isAbsolute;
        cLinkable* packets;
    };

    cInstructionParser (bool ipOptionalDestMAC);
    ~cInstructionParser ();
    void parse (const char* instruction, cResult& result, bool ignoreTrailingGarbage = false, bool noEthHeader = false);
    static int printProtocolList ();

#ifdef WITH_UNITTESTS
        static void unitTest ();
#endif


private:
    const char* parseTimestamp (const char* p, bool& hasTimestamp, uint64_t& timestamp, bool& isAbsolute);
    const char* parseProtocollIdentifier (const char* p, const char** identifier, size_t *len);

    // protocol specific parsers
    cLinkable* compileRAW  (bool noEthHeader, cParameterList& params);
    cLinkable* compileETH  (cParameterList& params);
    cLinkable* compileARP  (cParameterList& params, bool isProbe = false, bool isGratuitous = false);
    cLinkable* compileIP   (bool noEthHeader, cParameterList& params, bool isIPv6);
    cLinkable* compileUDP  (bool noEthHeader, cParameterList& params, bool isIPv6);
    cLinkable* compileVXLAN (bool noEthHeader, cParameterList& params, bool isIPv6);
    cLinkable* compileVRRP (bool noEthHeader, cParameterList& params, int version);
    cLinkable* compileSTP  (bool noEthHeader, cParameterList& params, bool isRSTP = false, bool isTCN = false);
    cLinkable* compileIGMP (bool noEthHeader, cParameterList& params, bool v3, bool query, bool report, bool leave);
    cLinkable* compileICMP (bool noEthHeader, cParameterList& params);
    cLinkable* compileICMPWithEmbedded (bool noEthHeader, cParameterList& params, uint8_t type);
    cLinkable* compileICMPRedirect (bool noEthHeader, cParameterList& params);
    cLinkable* compileICMPPing (bool noEthHeader, cParameterList& params, bool reply);
    cLinkable* compileTCP  (bool noEthHeader, cParameterList& params);
    cLinkable* compileTCPSYN  (bool noEthHeader, cParameterList& params);
    cLinkable* compileTCPSYNACK  (bool noEthHeader, cParameterList& params);
    cLinkable* compileTCPSYNACK2  (bool noEthHeader, cParameterList& params);
    cLinkable* compileTCPFIN  (bool noEthHeader, cParameterList& params);
    cLinkable* compileTCPFINACK  (bool noEthHeader, cParameterList& params);
    cLinkable* compileTCPFINACK2  (bool noEthHeader, cParameterList& params);
    cLinkable* compileTCPRST  (bool noEthHeader, cParameterList& params);
    cLinkable* compileGRE (bool noEthHeader, cParameterList& params, bool isIPv6);

    // helpers
    bool   compileMacHeader (cParameterList& params, cEthernetPacket* packet, bool noDestination, bool destIsOptional = false);
    size_t compileVLANTags  (cParameterList& params, cEthernetPacket* packet);
    bool   parseIPv4Params  (cParameterList& params, cIPPacket* packet, bool noDestinationIP = false);
    bool   parseIPv6Params  (cParameterList& params, cIPPacket* packet, bool noDestinationIP = false);
    const uint8_t* compileEmbedded  (cParameter* emb, bool noEthHeader, size_t& len);
    cMacAddress getParameterOrOwnMac (cParameterList& params, const char* par) const;
    cIPv4  getParameterOrOwnIPv4 (cParameterList& params, const char* par) const;
    cIPv6  getParameterOrOwnIPv6 (cParameterList& params, const char* par) const;


    void throwParseException (const char* msg, const char* val, size_t valLen = 0, const char* details = nullptr);

    const char* m_currentInstruction;
    bool        m_ipOptionalDestMAC;
    unsigned    m_recursionDepth;
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
