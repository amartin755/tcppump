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

#include "bug.hpp"
#include "console.hpp"
#include "compiler.hpp"
#include "instructionparser.hpp"
#include "parsehelper.hpp"
#include "formatexception.hpp"
#include "fileioexception.hpp"
#if HAVE_PCAP
#include "pcapfileio.hpp"
#endif


cCompiler::cCompiler (inputType t, const cMacAddress& mac, const cIpAddress& ip, const cTimeval& delay, unsigned delayScale, bool optDestMAC)
: type(t), ownMac(mac), ownIP(ip), defaultDelay(delay), defaultDelayScale(delayScale), ipOptionalDestMAC(optDestMAC),
  fileParser (defaultDelay.us()/defaultDelayScale, ownMac, ownIP, ipOptionalDestMAC)
{
}


cPacketData& cCompiler::operator<< (const std::list<std::string>& input)
{
    switch (type)
    {
    case PACKET:
        processPackets (input);
        break;
    case SCRIPT:
        processScriptFiles (input);
        break;
#if HAVE_PCAP
    case PCAP:
        processPcapFiles (input);
        break;
#endif
    default:
        BUG ("unkown input type");
    }
    return data;
}


void cCompiler::processPcapFiles (const std::list<std::string>& input)
{
    Console::PrintDebug ("Parsing %d PCAP files ...\n", input.size());

    for (const auto & file : input)
    {
        cPcapFileIO pcap;
        cTimeval t;
        int len;

        Console::PrintDebug ("Open '%s'\n", file.c_str());

        if (!pcap.open (file.c_str(), false))
        {
            throw FileIOException (FileIOException::OPEN, file.c_str());
        }

        uint8_t* data;

        while ((data = pcap.read(&t, &len)) != nullptr)
        {
            cEthernetPacket packet;
            packet.setRaw (data, len);

            this->data.packets.push_back (std::move(packet));
            this->data.timestamps.push_back (t);
        }
        if (pcap.error ())
        {
            throw FileIOException (FileIOException::READ, file.c_str());
        }

        pcap.close ();
    }
}


void cCompiler::processPackets (const std::list<std::string>& input)
{
    cInstructionParser::cResult result (data.packets);
    cTimeval timestamp, currtime;

    Console::PrintDebug ("Parsing %d packets ...\n", input.size());

    for (const auto & packet : input)
    {
        int count = cInstructionParser (ownMac, ownIP, ipOptionalDestMAC).parse (packet.c_str(), result);
        if (result.hasTimestamp)
        {
            data.hasUserTimestamps = true;
        }
        else
        {
            result.timestamp  = defaultDelay.us()/defaultDelayScale;
            result.isAbsolute = false;
        }
        timestamp.setUs(result.timestamp * defaultDelayScale);

        for (int n = 0; n < count; n++)
        {
            if (!result.isAbsolute)
            {
                data.timestamps.push_back (timestamp);
                currtime.add (timestamp);
            }
            else
            {
                if (timestamp < currtime)  // FIXME What to do if timestamp < currtime? delay = 0 or parse exception?
                    BUG ("FIXME");
                else
                {
                    cTimeval delta(timestamp);
                    data.timestamps.push_back (delta.sub (currtime));
                    currtime.set (timestamp);
                }
            }
        }
    }
}


void cCompiler::processScriptFiles (const std::list<std::string>& input)
{
    cInstructionParser::cResult result (data.packets);
    cTimeval timestamp, currtime, scriptStartTime;
    int count;

    Console::PrintDebug ("Parsing %d script files ...\n", input.size());

    for (const auto & file : input)
    {
        Console::PrintDebug ("Open '%s'\n", file.c_str());

        if (!fileParser.open (file.c_str()))
        {
            throw FileIOException (FileIOException::OPEN, file.c_str());
        }

        scriptStartTime = currtime;

        do
        {
            count = fileParser.parse (result);
            timestamp.setUs(result.timestamp * defaultDelayScale);
            if (result.hasTimestamp)
            {
                data.hasUserTimestamps = true;
            }

            for (int n = 0; n < count; n++)
            {
                if (!result.isAbsolute)
                {
                    data.timestamps.push_back (timestamp);
                    currtime.add (timestamp);
                }
                else
                {
                    timestamp.add(scriptStartTime);

                    if (timestamp < currtime) // FIXME What to do if timestamp < currtime? delay = 0 or parse exception?
                        BUG ("FIXME");
                    else
                    {
                        cTimeval delta(timestamp);
                        data.timestamps.push_back (delta.sub (currtime));
                        currtime.set (timestamp);
                    }
                }
            }

        } while (count > 0);

        fileParser.close ();

        if (count != EOF)
            throw FileIOException (FileIOException::READ, file.c_str());
    }
}
