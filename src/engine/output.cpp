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

#include <chrono>
#include <stdexcept>

#include "bug.hpp"
#include "output.hpp"

#include "fileioexception.hpp"
#include "console.hpp"
#include "signal.hpp"


cOutput::cOutput (const cPreprocessor &p)
: preproc (p), netif (nullptr), realtimeMode (false), repeat (1), pcapWrittenPackets(0), pcapWrittenBytes(0)
{
}

void cOutput::prepare(cInterface &netif, bool realtime, int repeat)
{
    this->netif  = &netif;
    realtimeMode = realtime;
    this->repeat = repeat;
}

#if HAVE_PCAP
void cOutput::prepare (const char* pcapOutFile, int repeat)
{
    realtimeMode = false;
    this->repeat = repeat;
    pcapWrittenPackets = 0;
    pcapWrittenBytes = 0;

    if (!outfile.open (pcapOutFile, true))
        throw FileIOException (FileIOException::OPEN, pcapOutFile);

}
#endif

cPacketData& cOutput::operator<< (cPacketData& input)
{
    cTimeval sendTime;
    bool endless = !repeat;


    if (netif)
    {
        netif->prepareSendQueue(input.getPacketCnt() * repeat,
                input.getTotalPacketBytes(),
                realtimeMode);
    }

    while (!cSignal::sigintSignalled() && (endless || repeat--))
    {
        for (cLinkable* p = input.getFirst(); p != nullptr; p = p->getNext())
        {
            sendTime.add (p->getTime());
            cEthernetPacket* eth;
            cIPv4Packet* ipv4;

            if ((eth = dynamic_cast<cEthernetPacket*>(p)) != nullptr)
            {
                processPacket (sendTime, *eth);
            }
            else if ((ipv4 = dynamic_cast<cIPv4Packet*>(p)) != nullptr)
            {
                std::list<cEthernetPacket>& packets = ipv4->getAllEthernetPackets();

                for (auto & p : packets)
                {
                    processPacket (sendTime, p);
                }
            }
        }
    }

    if (netif)
    {
        netif->flushSendQueue();
    }

    return input;
}

void cOutput::processPacket (const cTimeval& sendTime, cEthernetPacket& p)
{
    preproc.process (p);    // execute packet preprocessor hooks

    if (netif)
    {
        if(!netif->sendPacket (p.get(), p.getLength(), sendTime))
        {
            throw std::runtime_error("Could not send packet.");
        }
    }
#if HAVE_PCAP
    else
    {
        if (!outfile.write (sendTime, p.get(), (int)p.getLength(), true))
            throw FileIOException (FileIOException::WRITE, outfile.name());
        pcapWrittenPackets++;
        pcapWrittenBytes += (uint64_t)p.getLength ();
    }
#endif
}

void cOutput::statistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const
{
    if (netif)
    {
        netif->getSendStatistic (sentPackets, sentBytes, duration);
    }
    else
    {
        sentPackets = pcapWrittenPackets;
        sentBytes   = pcapWrittenBytes;
        duration    = 0;
    }
}
