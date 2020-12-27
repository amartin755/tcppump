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

#ifndef PACKETDATA_HPP_
#define PACKETDATA_HPP_

#include "ethernetpacket.hpp"
#include "ipv4packet.hpp"
#include "timeval.hpp"
#include "linkable.hpp"

class cPacketData
{
public:
    cPacketData ()
    {
        hasUserTimestamps = false;
        head = tail = nullptr;
        elements = ethPackets = ipv4Packets = totalBytes = 0;
    }

    ~cPacketData ()
    {
        for (cLinkable* p = getFirst(); p != nullptr; )
        {
            cLinkable* next = p->getNext();
            delete p;
            p = next;
        }

    }

    void add (cLinkable* packet)
    {
        if (!head)
        {
            head = packet;
        }
        else
        {
            tail->setNext(packet);
        }
        tail = packet;
        packet->setNext(nullptr);

        elements++;
    }

    void addPacket (cLinkable* packet)
    {
        add (packet);
        updateStats (packet);
    }

    cLinkable* getFirst (void)
    {
        return head;
    }

    cLinkable* getLast (void)
    {
        return tail;
    }

    size_t getPacketCnt (void) const
    {
        return ethPackets + ipv4Packets;
    }
    size_t getTotalPacketBytes (void) const
    {
        return totalBytes;
    }

    // TODO encapsulate
    bool hasUserTimestamps; // is true, if at least one timestamp in timestamp list is user defined (no default value)



private:

    // FIXME  byte and packet counting will not work anymore when we have loops
    void updateStats (cLinkable* packet)
    {
        cEthernetPacket* eth = dynamic_cast<cEthernetPacket*>(packet);
        if (eth)
        {
            ethPackets++;
            totalBytes += eth->getLength();
        }
        else
        {
            cIPv4Packet* ipv4 = dynamic_cast<cIPv4Packet*>(packet);
            BUG_ON (ipv4);

            ipv4Packets++;
            const std::list<cEthernetPacket>& fragments = ipv4->getAllEthernetPackets();

            for (auto & p : fragments)
            {
                ethPackets++;
                totalBytes += p.getLength();
            }
        }
    }
    cLinkable* head;
    cLinkable* tail;
    size_t elements;
    size_t ethPackets;
    size_t ipv4Packets;
    size_t totalBytes;
};

#endif /* PACKETDATA_HPP_ */
