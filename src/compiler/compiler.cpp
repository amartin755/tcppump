// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2024 Andreas Martin (netnag@mailbox.org)
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

#include <chrono>

#include "bug.hpp"
#include "console.hpp"
#include "compiler.hpp"
#include "instructionparser.hpp"
#include "parsehelper.hpp"
#include "formatexception.hpp"
#include "fileioexception.hpp"
#include "pcapfileio.hpp"


cCompiler::cCompiler (inputType t, const cTimeval& delay, unsigned delayScale, bool optDestMAC, double pcapScaling)
: type(t), defaultDelay(delay), defaultDelayScale(delayScale), ipOptionalDestMAC(optDestMAC),
  fileParser (defaultDelay.us()/defaultDelayScale, ipOptionalDestMAC), pcapScalingFactor(pcapScaling)
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
    Console::PrintDebug ("Parsing %d PCAP files ...\n", (int)input.size());

    cTimeval currtime;
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
        data.hasUserTimestamps = pcapScalingFactor == 0 ? false : true;

        while ((pcapdata = pcap.read(&t, &len)) != nullptr)
        {
            cEthernetPacket* packet = new cEthernetPacket(len);
            packet->setRaw (pcapdata, len);
            cTimeval delta(t);
            packet->setTime (delta.sub (currtime).mul(pcapScalingFactor));
            currtime.set(t);

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

    Console::PrintDebug ("Parsing %d packets ...\n", (int)input.size());

    for (const auto & packet : input)
    {
        cInstructionParser (ipOptionalDestMAC).parse (packet.c_str(), result);
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

    Console::PrintDebug ("Parsing %d script files ...\n", (int)input.size());

    for (const auto & file : input)
    {
        Console::PrintDebug ("Open '%s'\n", file.c_str());

        if (!fileParser.open (file.c_str()))
        {
            throw FileIOException (FileIOException::OPEN, file.c_str());
        }

        scriptStartTime = currtime;
        auto t1 = std::chrono::high_resolution_clock::now();

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
        auto t2 = std::chrono::high_resolution_clock::now();
        auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        Console::PrintDebug ("parsed %zu packets in %.2f usec\n", data.getPacketCnt(), (double)elapsedUs.count());

        fileParser.close ();

        if (count != EOF)
            throw FileIOException (FileIOException::READ, file.c_str());
    }
}
