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
#include <cassert>
#include <cctype>
#include <cstring>

#include "fileparser.hpp"

#include "instructionparser.hpp"
#include "ethernetpacket.hpp"


cFileParser::cFileParser ()
{
    instructionBufferSize = 0;
    instructionBuffer     = NULL;

    fp           = NULL;
    lineNbr      = 1;
    lastError[0] = '\0';
    delay        = 0;
}

cFileParser::~cFileParser ()
{
    free (instructionBuffer);
}

void cFileParser::init (FILE* fp, uint64_t defaultDelay, const cMacAddress& ownMac, const cIpAddress& ownIPv4)
{
    assert (fp);

    this->fp      = fp;

    lineNbr       =  1;
    lastError[0]  = '\0';

    delay         = defaultDelay;
    this->ownMac.set (ownMac);
    this->ownIPv4.set (ownIPv4);
}

/**
 *  Each call delivers exactly one parsed instruction, which can result in one or more ethernet packets.
 *  It returns the number of packets that where added to the list or EOF/PARSE_ERROR.
 */
int cFileParser::parse (uint64_t& timestamp, bool& isAbsolute, std::list <cEthernetPacket> &packets)
{
    int offset = 0;
    int c;
    bool comment = false;

    while ((c = getc (fp)) != EOF)
    {
        if (offset >= instructionBufferSize)
        {
            instructionBufferSize += 10*1024;
            void* newbuf = realloc (instructionBuffer, instructionBufferSize);
            if (newbuf)
            {
                instructionBuffer = (char*)newbuf;
            }
            else
            {
                return parseError ("Could not allocate memory! Line too long?!");
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
                        isAbsolute = false;
                        timestamp  = delay;
                        try
                        {
                            return cInstructionParser (ownMac, ownIPv4)
                                    .parse (instructionBuffer, timestamp, isAbsolute, packets);
                        }
                        catch (ParseException &e)
                        {
                            return parseError (e.what (), e.value ());
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

int cFileParser::parseError (const char* errmsg, const char* pos)
{
    const char* position = "";
    if (pos)
        position = pos;
    snprintf (lastError, sizeof (lastError), "(line %u) %s\n\t %s", lineNbr, errmsg, position);
    if (pos)
        strncat (lastError, "\n\t^", sizeof (lastError));
    return PARSE_ERROR;
}

const char* cFileParser::getLastError ()
{
    return lastError;
}

