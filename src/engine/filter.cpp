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


#include "filter.hpp"
#include "console.hpp"

cFilter::cFilter (const cMacAddress* ovrDMAC, const cMacAddress* drDMAC,
        const cMacAddress* drSMAC) : forcedDMAC(ovrDMAC), dropDMAC(drDMAC), dropSMAC(drSMAC)
{
}


cPacketData& cFilter::operator<< (cPacketData& input)
{
    if (forcedDMAC || dropDMAC || dropSMAC) // only walk through the list, if a filter is active
    {
        Console::PrintDebug ("Filtering ...\n");

        for (cLinkable* p = input.getFirst(); p != nullptr; p = p->getNext())
        {
            if (forcedDMAC)
            {
                cEthernetPacket* eth;
                cIPPacket*     ipv4;

                if ((eth = dynamic_cast<cEthernetPacket*>(p)) != nullptr)
                {
                    eth->setDestMac (*forcedDMAC);
                }
                else if ((ipv4 = dynamic_cast<cIPPacket*>(p)) != nullptr)
                {
                    ipv4->setDestMac (*forcedDMAC);
                }
            }

            // TODO add more filters
        }
    }
    return input;
}

