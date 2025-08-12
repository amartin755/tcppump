// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2025 Andreas Martin (netnag@mailbox.org)
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
#include <stdexcept>

#include "bug.hpp"
#include "output.hpp"

#include "fileioexception.hpp"
#include "console.hpp"
#include "signal.hpp"
#include "pcapbackend.hpp"
#include "asciibackend.hpp"


cOutput::cOutput (const cPreprocessor &p)
: m_outfile (nullptr), m_preproc (p), m_netif (nullptr), m_realtimeMode (false), m_repeat (1)
{
}

cOutput::~cOutput ()
{
    if (m_outfile)
        delete m_outfile;

    m_outfile = nullptr;
}

void cOutput::prepare(cNetInterface &netif, bool realtime, int repeat)
{
    m_netif  = &netif;
    m_realtimeMode = realtime;
    m_repeat = repeat;
}

void cOutput::prepare (const char* file, const char* format, int repeat)
{
    std::string fileFormat(format);
    m_realtimeMode = false;
    m_repeat = repeat;

    if (fileFormat == "pcap")
    {
        m_outfile = new cPcapBackend (file);
    }
    else if (fileFormat == "text")
    {
        m_outfile = new cAsciiBackend (file, true, true, false, "\t", " ");
    }
    else if (fileFormat == "hexstream")
    {
        m_outfile = new cAsciiBackend (file, false, false, false, "", "");
    }
    else if (fileFormat == "hexdump")
    {
        m_outfile = new cAsciiBackend (file, false, false, true, "\t", "");
    }
    else
    {
        throw FileIOException (FileIOException::FORMAT, format);
    }
}

cPacketData& cOutput::operator<< (cPacketData& input)
{
    cTimeval sendTime;
    bool endless = !m_repeat;
    bool queuedOutput = m_netif;


    if (queuedOutput)
    {
        m_netif->prepareSendQueue(input.getPacketCnt() * m_repeat,
                input.getTotalPacketBytes() * m_repeat,
                m_realtimeMode);
    }

    while (!cSignal::sigintSignalled() && (endless || m_repeat--))
    {
        for (cLinkable* p = input.getFirst(); !cSignal::sigintSignalled() && (p != nullptr); p = p->getNext())
        {
            sendTime.add (p->getTime());
            cEthernetPacket* eth;
            cIPPacket* ipv4;

            if ((eth = dynamic_cast<cEthernetPacket*>(p)) != nullptr)
            {
                processPacket (sendTime, *eth);
            }
            else if ((ipv4 = dynamic_cast<cIPPacket*>(p)) != nullptr)
            {
                std::list<cEthernetPacket>& packets = ipv4->getAllEthernetPackets();

                for (auto & currPacket : packets)
                {
                    processPacket (sendTime, currPacket);
                }
            }
        }
    }

    if (queuedOutput)
    {
        m_netif->flushSendQueue();
    }

    return input;
}

void cOutput::processPacket (const cTimeval& sendTime, cEthernetPacket& p)
{
    m_preproc.process (p);    // execute packet preprocessor hooks

    if (m_netif)
    {
        if(!m_netif->sendPacket (p.get(), p.getLength(), sendTime))
        {
            throw std::runtime_error("Could not send packet.");
        }
    }
    else
    {
        m_outfile->write (sendTime, p);
    }
}

void cOutput::statistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const
{
    if (m_netif)
    {
        m_netif->getSendStatistic (sentPackets, sentBytes, duration);
    }
    else
    {
        m_outfile->statistic (sentPackets, sentBytes, duration);
    }
}
