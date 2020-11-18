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


#include <cstdio>
#include <cctype>
#include <cstring>

#include "fileparser.hpp"

#include "bug.hpp"
#include "instructionparser.hpp"
#include "ethernetpacket.hpp"


cFileParser::cFileParser (uint64_t defaultDelay, const cMacAddress& ownMac, const cIpAddress&  ownIPv4, bool ipOptionalDestMAC)
{
    instructionBufferSize = 0;
    instructionBuffer     = nullptr;
    fp                    = nullptr;
    lineNbr               = 1;
    delay                 = defaultDelay;
    path                  = nullptr;

    this->ownMac .set (ownMac);
    this->ownIPv4.set (ownIPv4);
    this->ipOptionalDestMAC = ipOptionalDestMAC;
}

cFileParser::~cFileParser ()
{
    std::free (instructionBuffer);
    close ();
}

bool cFileParser::open (const char* path)
{
    BUG_ON (!fp);

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
    BUG_ON (fp);

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
            if (comment)
            {
                if (c == '\n' || c == '\r')
                {
                    lineNbr++;
                    comment = false;
                }
            }
            else
            {
                if (c == '\n' || c == '\r')
                {
                    lineNbr++;
                    c = ' ';
                }
                else
                {
                    // instructions are terminated with ';'
                    if (c == ';')
                    {
                        instructionBuffer[offset++] = '\0';
                        result.isAbsolute = false;
                        result.timestamp  = delay;
                        try
                        {
                            return cInstructionParser (ownMac, ownIPv4, ipOptionalDestMAC)
                                    .parse (instructionBuffer, result);
                        }
                        catch (ParseException &e)
                        {
                            throw FileParseException (path, lineNbr, e.instruction(), e.errorMsg(), e.details(), e.errorBegin(), e.errorLen());
                        }
                    }
                    else
                    {
                        instructionBuffer[offset++] = c;
                    }
                }

            }
        }
    }

    return EOF;
}
