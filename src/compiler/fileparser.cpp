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


#include <cstdio>
#include <cctype>
#include <cstring>

#include "fileparser.hpp"

#include "bug.hpp"
#include "console.hpp"
#include "instructionparser.hpp"
#include "ethernetpacket.hpp"
#include "parsehelper.hpp"


cFileParser::cFileParser (uint64_t defaultDelay, bool ipOptionalDestMAC)
{
    instructionBufferSize = 0;
    instructionBuffer     = nullptr;
    fp                    = nullptr;
    lineNbr               = 1;
    delay                 = defaultDelay;
    path                  = nullptr;
    openControlBlocks     = 0;

    this->ipOptionalDestMAC = ipOptionalDestMAC;
}

cFileParser::~cFileParser ()
{
    std::free (instructionBuffer);
    close ();
}

bool cFileParser::open (const char* path)
{
    BUG_ON (fp);

    if ((fp = std::fopen (path, "rt")) == NULL)
    {
        return false;
    }

    lineNbr    =  1;
    this->path = path;

    return true;
}

void cFileParser::close (void)
{
    if (fp)
    {
        std::fclose(fp);
        fp   = nullptr;
        path = nullptr;
    }
}

/**
 *  Each call delivers exactly one parsed instruction, which can result in one or more ethernet packets.
 *  It returns the number of packets that where added to the list or EOF/PARSE_ERROR.
 */
int cFileParser::parse (cInstructionParser::cResult& result)
{
    BUG_ON (!fp);

    int offset = 0;
    int c;
    bool comment = false;

    while ((c = std::getc (fp)) != EOF)
    {
        if (offset >= instructionBufferSize)
        {
            instructionBufferSize += 10*1024;
            void* newbuf = std::realloc (instructionBuffer, instructionBufferSize);
            if (newbuf)
            {
                instructionBuffer = (char*)newbuf;
            }
            else
            {
                // Could not allocate memory! Line too long?!
                throw std::bad_alloc();
            }
        }

        if (c == '#')
        {
            comment = true;
        }
        else
        {
            if (c == '\n')
            {
                lineNbr++;
            }
            if (comment)
            {
                if (c == '\n')
                {
                    comment = false;
                }
            }

            if (!comment)
            {
                // instructions are terminated with ';'
                if (c == ';')
                {
                    instructionBuffer[offset++] = '\0';
                    result.isAbsolute = false;
                    result.timestamp  = delay;
                    try
                    {
                        cInstructionParser (ipOptionalDestMAC)
                                .parse (instructionBuffer, result);
                        return 0;
                    }
                    catch (ParseException &e)
                    {
                        throw FileParseException (path, lineNbr, e.instruction(), e.errorMsg(), e.details(), e.errorBegin(), e.errorLen());
                    }
                }
                // control blocks start with '{'
                else if (c == '{')
                {
                    openControlBlocks++;
                    Console::PrintDebug("## block start %d\n", lineNbr);
                    // TODO
                    // - parse timestamp
                    // - parse control key word and its parameters

                    offset = 0;
                }
                // control blocks are terminated with '}'
                else if (c == '}')
                {
                    openControlBlocks--;
                    instructionBuffer[offset++] = c;
                    Console::PrintDebug("## block end %d\n", lineNbr);
                    const char* p = cParseHelper::skipWhitespaces (instructionBuffer);
                    if (*p != '}')
                        throw FileParseException (path, lineNbr, instructionBuffer, "syntax error before '}'. Missing ';'?", nullptr, p, 0);

                    if (openControlBlocks < 0)
                        throw FileParseException (path, lineNbr, instructionBuffer, "Missing matching ‘{’", nullptr, p, 0);

                    // TODO
                    // generate jump label that points to start of the block

                    offset = 0;
                }
                else
                {
                    instructionBuffer[offset++] = c;
                }
            }
        }
    }

    BUG_ON (openControlBlocks < 0); // must be already handled above

    if (openControlBlocks > 0)
        throw FileParseException (path, lineNbr, nullptr, "Expected ‘}’ at end of input", nullptr, nullptr, 0);

    return EOF;
}
