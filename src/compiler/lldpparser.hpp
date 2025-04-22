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
#ifndef LLDP_PARSER_HPP_
#define LLDP_PARSER_HPP_

#include "lldppacket.hpp"
#include "parameterlist.hpp"

class cLldpParser
{
public:
    cLldpParser (cLldpPacket&, cParameterList&);
    void chassisID ();
    void portID ();
    void portDescription ();
    void systemName ();
    void systemDescription ();
    void systemCapabilities ();
    void managementAddress ();

    // IEEE802.1Q TLVs
    void portVID ();
    void portProtocolVID ();
    void vlanName ();
    void protocolIdentity ();
    void vidUsageDigest ();
    void managementVID ();
    void linkAggregation ();
    void congestionNotification ();
    void etsConfiguration ();
    void etsRecommendation ();
    void pfcControlConfig ();
    void applicationPriority ();
    void evb ();
    void cdcb ();
    void applicationVLAN ();

    // IEEE802.3 TLVs
    void macPhyConfigStatus ();
    void powerViaMDI ();
    void maxFrameSize ();
    void eee ();
    void eeeFastWake ();

    // Profinet TLVs
    void pnDelay ();
    void pnPortStatus ();
    void pnAlias ();
    void pnMrpPortState ();
    void pnChassisMac ();
    void pnPTCPStatus ();
    void pnMAUTypeExtension ();
    void pnMrpIcPortStatus ();

    void allRawTLVs ();
    void allOidTLVs ();

private:
    cLldpPacket& m_packet;
    cParameterList& m_params;
};

#endif /* LLDP_PARSER_HPP_ */