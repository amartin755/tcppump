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
#include "pcapfileio.hpp"


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
    case PCAP:
        processPcapFiles (input);
        break;
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

        uint8_t* pcapdata;

        while ((pcapdata = pcap.read(&t, &len)) != nullptr)
        {
            cEthernetPacket* packet = new cEthernetPacket(len);
            packet->setRaw (pcapdata, len);
            packet->setTime (t);

            data.addPacket(packet);
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
    cInstructionParser::cResult result;
    cTimeval timestamp, currtime;

    Console::PrintDebug ("Parsing %d packets ...\n", input.size());

    for (const auto & packet : input)
    {
        cInstructionParser (ownMac, ownIP, ipOptionalDestMAC).parse (packet.c_str(), result);
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

        if (!result.isAbsolute)
        {
            result.packets->setTime(timestamp);
            data.addPacket(result.packets);
            currtime.add (timestamp);
        }
        else
        {
            if (timestamp < currtime)  // FIXME What to do if timestamp < currtime? delay = 0 or parse exception?
                BUG ("FIXME");
            else
            {
                cTimeval delta(timestamp);
                result.packets->setTime(delta.sub (currtime));
                data.addPacket(result.packets);
                currtime.set (timestamp);
            }
        }
    }
}


void cCompiler::processScriptFiles (const std::list<std::string>& input)
{
    cInstructionParser::cResult result;
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

        while ((count = fileParser.parse (result)) >= 0)
        {
            timestamp.setUs(result.timestamp * defaultDelayScale);
            if (result.hasTimestamp)
            {
                data.hasUserTimestamps = true;
            }

            if (!result.isAbsolute)
            {
                result.packets->setTime(timestamp);
                data.addPacket(result.packets);
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
                    result.packets->setTime(delta.sub (currtime));
                    data.addPacket(result.packets);
                    currtime.set (timestamp);
                }
            }
        }

        fileParser.close ();

        if (count != EOF)
            throw FileIOException (FileIOException::READ, file.c_str());
    }
}
