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


#include <cstdint>

#include "responder.hpp"
#include "ethernetpacket.hpp"
#include "ipv4packet.hpp"
#include "udppacket.hpp"
#include "tcppacket.hpp"
#include "signal.hpp"
#include "console.hpp"

cResponder::cResponder (cNetInterface& ifc) : netif (ifc)
{
}

void cResponder::mirror (void)
{
    int len;
    cEthernetPacket packet;
    const uint8_t* p;
    cMacAddress srcMac, dstMac, ourMac;
    cTimeval t;

    netif.getMAC (ourMac);

    while (!cSignal::sigintSignalled() && (p = netif.receivePacket(nullptr, &len)))
    {
        try
        {
            packet.setRaw(p, len);

            packet.getDestMac(dstMac);
            if (dstMac.isUnicast())
            {
                // exchange src and dest mac address
                packet.getSrcMac  (srcMac);
                packet.setDestMac (srcMac);
                packet.setSrcMac  (dstMac);
            }
            else
            {
                // keep dest mac, use own mac as source
                packet.setSrcMac (ourMac);
            }

            if (packet.hasLlcHeader())
            {
                // exchange dsap <-> ssap
                // TODO
            }
            else
            {
                if (packet.getTypeLength() == ETHERTYPE_IPV4)
                {
                    const ipv4_header_t* ipHeader = (ipv4_header_t*)packet.getPayload ();
                    cIPv4 dstIP(ipHeader->dstIp);
                    if (!dstIP.isMulticast())
                    {
                        // exchange src and dest IP address
                        cIPv4 srcIP(ipHeader->srcIp);
                        packet.updatePayloadAt (offsetof (ipv4_header_t, dstIp), srcIP.getAsArray(), 4);
                        packet.updatePayloadAt (offsetof (ipv4_header_t, srcIp), dstIP.getAsArray(), 4);
                    }
                    else
                    {
                        // else --> keep dest ip, use own IP as source --> ip checksum must be recalculated
                        // TODO
                    }

                    // if UDP or TCP --> swap sport and dport
                    if (ipHeader->protocol == cIPv4Packet::protocols::PROTO_UDP)
                    {
                        unsigned udpHeaderOffset = ipHeader->getHeaderLenght() * 4;
                        const udp_header_t *udpHeader = reinterpret_cast <const udp_header_t *>(reinterpret_cast<const uint8_t*>(ipHeader) + udpHeaderOffset);
                        uint16_t srcPort = udpHeader->srcPort;
                        uint16_t dstPort = udpHeader->dstPort;

                        packet.updatePayloadAt (udpHeaderOffset + offsetof (udp_header_t, dstPort), &srcPort, sizeof (srcPort));
                        packet.updatePayloadAt (udpHeaderOffset + offsetof (udp_header_t, srcPort), &dstPort, sizeof (dstPort));
                    }
                    else if (ipHeader->protocol == cIPv4Packet::protocols::PROTO_TCP)
                    {
                        unsigned tcpHeaderOffset = ipHeader->getHeaderLenght() * 4;
                        const tcp_header_t *tcpHeader = reinterpret_cast <const tcp_header_t *>(reinterpret_cast<const uint8_t*>(ipHeader) + tcpHeaderOffset);
                        uint16_t srcPort = tcpHeader->srcPort;
                        uint16_t dstPort = tcpHeader->dstPort;

                        packet.updatePayloadAt (tcpHeaderOffset + offsetof (tcp_header_t, dstPort), &srcPort, sizeof (srcPort));
                        packet.updatePayloadAt (tcpHeaderOffset + offsetof (tcp_header_t, srcPort), &dstPort, sizeof (dstPort));
                    }
                }
            }

            if (!netif.sendPacket(packet.get(), packet.getLength(), t))
                Console::PrintError ("Error while sending packet\n");
        }
        catch(FormatException &)
        {
            // ignore malformed packets
            Console::PrintDebug ("Ignoring malformed packet\n");
        }
    }
}
