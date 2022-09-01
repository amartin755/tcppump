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


#ifndef FILEPARSER_HPP_
#define FILEPARSER_HPP_

#include <cstdint>
#include <cstdio>
#include <list>
#include <stack>

#include "ethernetpacket.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "instructionparser.hpp"

const int PARSE_ERROR = -100;

class cParameterList;
class cLoop;

class cFileParser
{
public:
    cFileParser (uint64_t defaultDelay, bool ipOptionalDestMAC);
    ~cFileParser ();
    bool open (const char* path);
    int parse (cInstructionParser::cResult& result);
    void close (void);


private:

    char*  instructionBuffer;
    int    instructionBufferSize;

    uint64_t     delay;
    bool         ipOptionalDestMAC;
    FILE*        fp;
    const char*  path;

    unsigned     lineNbr;
    int          openControlBlocks;
    std::stack<cLoop*> loopstack;
};

class FileParseException : public ParseException
{
public:
    FileParseException (const char* file, int lineNbr, const char* inst, const char* errMsg, const char* details, const char* errBegin, int errLen) :
        ParseException(inst, errMsg, details, errBegin, errLen)
    {
        this->lineNbr = lineNbr;
        this->file    = file;
    }
    int lineNumber () const
    {
        return lineNbr;
    }
    const char* filePath () const
    {
        return file;
    }
private:
    int lineNbr;
    const char* file;
};

#endif /* FILEPARSER_HPP_ */
