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

#include <cstring>

#include "stppacket.hpp"
#include "bug.h"


cStpPacket::cStpPacket ()
{
}

void cStpPacket::compileConfigPdu (unsigned rootBridgePrio, unsigned rootBridgeId, const cMacAddress& rootBridgeMac, uint32_t pathCost,
        unsigned bridgePrio, unsigned bridgeId, const cMacAddress& bridgeMac, unsigned portPrio, unsigned portNumber,
        double msgAge, double maxAge, double helloTime, double forwardDelay, int flags)
{
    stp_bpdu_t bpdu;

    compileConfigPdu (bpdu, rootBridgePrio, rootBridgeId, rootBridgeMac, pathCost,
            bridgePrio, bridgeId, bridgeMac, portPrio, portNumber, msgAge, maxAge, helloTime, forwardDelay, flags);

    setPayload ((uint8_t*)&bpdu, sizeof (bpdu));
    setLength ();
}

void cStpPacket::compileConfigPduRstp (unsigned rootBridgePrio, unsigned rootBridgeId, const cMacAddress& rootBridgeMac, uint32_t pathCost,
        unsigned bridgePrio, unsigned bridgeId, const cMacAddress& bridgeMac, unsigned portPrio, unsigned portNumber,
        double msgAge, double maxAge, double helloTime, double forwardDelay, int flags, unsigned portRole)
{
    rstp_bpdu_t bpdu;

    compileConfigPdu (bpdu.stp, rootBridgePrio, rootBridgeId, rootBridgeMac, pathCost,
            bridgePrio, bridgeId, bridgeMac, portPrio, portNumber, msgAge, maxAge, helloTime, forwardDelay, flags);
    bpdu.stp.version = 2;
    bpdu.stp.type    = 2;
    bpdu.version1Len = 0;
    bpdu.stp.setRstpFlags (flags & PROPOSAL, flags & FORWARDING, flags & LEARNING, flags & AGREEMENT, portRole);

    setPayload ((uint8_t*)&bpdu, sizeof (bpdu));
    setLength ();
}

void cStpPacket::compileTcnPdu (void)
{
    prepareMacHeader ();
    tcnpdu_t tcn;

    tcn.protocol = 0;
    tcn.version  = 0;
    tcn.type     = 0x80;
    setPayload ((uint8_t*)&tcn, sizeof (tcn));
    setLength ();
}

uint16_t cStpPacket::toTime (double seconds) const
{
    return (uint16_t)(seconds / (1.0/256.0));
}

void cStpPacket::prepareMacHeader (void)
{
    // set Ethernet header destination multicast mac address 01:80:C2:00:00:0
    this->setDestMac (cMacAddress(1, 0x80, 0xc2, 0, 0, 0));

    // 802.2 header
    addLlcHeader (0x42, 0x42, 3);
}

inline void cStpPacket::compileConfigPdu (stp_bpdu_t& bpdu, unsigned rootBridgePrio, unsigned rootBridgeId, const cMacAddress& rootBridgeMac, uint32_t pathCost,
        unsigned bridgePrio, unsigned bridgeId, const cMacAddress& bridgeMac, unsigned portPrio, unsigned portNumber,
        double msgAge, double maxAge, double helloTime, double forwardDelay, int flags)
{
    prepareMacHeader ();

    std::memset (&bpdu, 0, sizeof (bpdu));

    bpdu.root.set (rootBridgePrio, rootBridgeId, rootBridgeMac);
    bpdu.rootPathCost = htonl (pathCost);
    bpdu.bridge.set (bridgePrio, bridgeId, bridgeMac);
    bpdu.setPortId (portPrio, portNumber);
    bpdu.messageAge = htons (toTime (msgAge));
    bpdu.maxAge = htons (toTime (maxAge));
    bpdu.forwardDelay = htons (toTime (forwardDelay));
    bpdu.helloTime = htons (toTime (helloTime));
    bpdu.setStpFlags (flags & TOPO_CHANGE, flags & TOPO_CHANGE_ACK);
}
