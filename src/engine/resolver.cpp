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
#include <utility>
#include "resolver.hpp"
#include "console.hpp"
#include "macaddress.hpp"
#include "ipv4packet.hpp"

cResolver::cResolver (cInterface &netif) : arper (netif)
{
    // TODO Auto-generated constructor stub

}

cPacketData& cResolver::operator<< (cPacketData& input)
{
    Console::PrintDebug ("Resolving ...\n");

    for (auto & p : input.packets)
    {
        if (!p.hasDestMac() && p.getEthertype() == ntohs(ETHERTYPE_IPV4))
        {
            cMacAddress dmac;
            const ipv4_header_t* ipheader = (const ipv4_header_t*)p.getPayload ();
            cIpAddress dip(ipheader->dstIp);

            try
            {
                dmac = cache.at (dip);
            }
            catch (const std::out_of_range&)
            {
                std::string sIP;
                dip.get (sIP);

                Console::PrintMostVerbose ("Try to resolve MAC of host %s ... ", sIP.c_str());
                if (arper.resolve(dip, dmac))
                {
                    Console::PrintMostVerbose (" is at %02x:%02x:%02x:%02x:%02x:%02x \n",
                            ((const uint8_t*)dmac.get())[0],
                            ((const uint8_t*)dmac.get())[1],
                            ((const uint8_t*)dmac.get())[2],
                            ((const uint8_t*)dmac.get())[3],
                            ((const uint8_t*)dmac.get())[4],
                            ((const uint8_t*)dmac.get())[5]);
                    cache.insert ({dip, dmac});
                }
                else
                {
                    Console::PrintError ("TIMEOUT! %s unreachable\n", sIP.c_str());
                    throw std::runtime_error("Could not resolve host(s).");
                }
            }
            p.setDestMac (dmac);
        }
    }

    return input;
}
