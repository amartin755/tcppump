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


#ifndef FILEPARSER_HPP_
#define FILEPARSER_HPP_

#include <cstdint>
#include <cstdio>
#include <list>

#include "ethernetpacket.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "instructionparser.hpp"

const int PARSE_ERROR = -100;

class cParameterList;

class cFileParser
{
public:
    cFileParser ();
    ~cFileParser ();
    void init (FILE* fp, uint64_t defaultDelay, const cMacAddress& ownMac, const cIpAddress&  ownIPv4);
    int parse (cInstructionParser::cResult& result);


private:

    char*  instructionBuffer;
    int    instructionBufferSize;

    uint64_t     delay;
    cMacAddress  ownMac;
    cIpAddress   ownIPv4;
    FILE*        fp;

    unsigned     lineNbr;
};

class FileParseException : public ParseException
{
public:
    FileParseException (int lineNbr, const char* inst, const char* errMsg, const char* details, const char* errBegin, int errLen) :
        ParseException(inst, errMsg, details, errBegin, errLen)
    {
        this->lineNbr = lineNbr;
    }
    int lineNumber () const
    {
        return lineNbr;
    }
private:
    int lineNbr;
};

#endif /* FILEPARSER_HPP_ */
