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

#include <sstream>

#include "lldpparser.hpp"
#include "syntax.hpp"
#include "settings.hpp"
#include "instructionparser.hpp"
#include "uuid.hpp"
#include "md5.hpp"


cLldpParser::cLldpParser (cLldpPacket& packet, cParameterList& params)
: m_packet (packet), m_params (params)
{

}


void cLldpParser::chassisID ()
{
    cParameter* optionalPar;
    size_t len;


    optionalPar = m_params.findParameter (PAR_LLDP_CHASSIS_ID_T.syntax, true);
    if (optionalPar)
    {
        // User defined subtype. Chassis ID MUST be provided too
        const uint8_t* chassisID = m_params.findParameter (PAR_LLDP_CHASSIS_ID.syntax)->asStream (len, 255);
        uint8_t subtype = optionalPar->asInt8 ();
        m_packet.addChassisID (subtype, chassisID, (uint8_t)len);
    }
    else
    {
        // no subtpye provided. check if user provided chassis ID
        optionalPar = m_params.findParameter (PAR_LLDP_CHASSIS_ID.syntax, true);
        if (optionalPar)
        {
            // we accept IPv4, IPv6, MAC and bytestream.
            // try to find out the type the user provided
            try
            {
                m_packet.addChassisID(optionalPar->asIPv4());
            }
            catch (const FormatException&)
            {
                try
                {
                    m_packet.addChassisID(optionalPar->asIPv6());
                }
                catch (const FormatException&)
                {
                    try
                    {
                        m_packet.addChassisID(optionalPar->asMac());
                    }
                    catch (const FormatException&)
                    {
                        // if the type isn't IPv4, IPv6 or MAC, it MUST be a valid bytestream
                        const uint8_t* chassisID = optionalPar->asStream (len, 255);
                        m_packet.addChassisID (7, chassisID, (uint8_t)len);
                    }
                }
            }
        }
        else
        {
            // create default ChassisID with our own MAC address
            m_packet.addChassisID (cSettings::get().getMyMAC());
        }
    }
}

void cLldpParser::portID ()
{
    cParameter* optionalPar;
    size_t len;

    optionalPar = m_params.findParameter (PAR_LLDP_PORT_ID_T.syntax, true);
    if (optionalPar)
    {
        // User defined subtype. Port ID MUST be provided too
        const uint8_t* chassisID = m_params.findParameter (PAR_LLDP_PORT_ID.syntax)->asStream (len, 255);
        uint8_t subtype = optionalPar->asInt8 ();
        m_packet.addPortID (subtype, chassisID, (uint8_t)len);
    }
    else
    {
        // no subtpye provided. check if user provided port ID
        optionalPar = m_params.findParameter (PAR_LLDP_PORT_ID.syntax, true);
        if (optionalPar)
        {
            // we accept IPv4, IPv6, MAC and bytestream.
            // try to find out the type the user provided
            try
            {
                m_packet.addPortID(optionalPar->asIPv4());
            }
            catch (const FormatException&)
            {
                try
                {
                    m_packet.addPortID(optionalPar->asIPv6());
                }
                catch (const FormatException&)
                {
                    try
                    {
                        m_packet.addPortID(optionalPar->asMac());
                    }
                    catch (const FormatException&)
                    {
                        // if the type isn't IPv4, IPv6 or MAC, it MUST be a valid bytestream
                        const uint8_t* chassisID = optionalPar->asStream (len, 255);
                        m_packet.addPortID (7, chassisID, (uint8_t)len);
                    }
                }
            }
        }
        else
        {
            // create default ChassisID with our own MAC address
            m_packet.addPortID (cSettings::get().getMyMAC());
        }
    }
}

void cLldpParser::systemCapabilities ()
{
    uint16_t c = 0, e = 0;
    c =  m_params.findParameter (PAR_LLDP_SYSCAP_OTHER.syntax,       (uint32_t)0)->asInt16(0, 1)       |
        (m_params.findParameter (PAR_LLDP_SYSCAP_REPEATER.syntax,    (uint32_t)0)->asInt16(0, 1) << 1) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_BRIDGE.syntax,      (uint32_t)0)->asInt16(0, 1) << 2) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_WLAN.syntax,        (uint32_t)0)->asInt16(0, 1) << 3) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_ROUTER.syntax,      (uint32_t)0)->asInt16(0, 1) << 4) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_PHONE.syntax,       (uint32_t)0)->asInt16(0, 1) << 5) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_DOCSIS.syntax,      (uint32_t)0)->asInt16(0, 1) << 6) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_STATION.syntax,     (uint32_t)1)->asInt16(0, 1) << 7);
    e =  m_params.findParameter (PAR_LLDP_SYSCAP_OTHER_EN.syntax,    (uint32_t)0)->asInt16(0, 1)       |
        (m_params.findParameter (PAR_LLDP_SYSCAP_REPEATER_EN.syntax, (uint32_t)0)->asInt16(0, 1) << 1) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_BRIDGE_EN.syntax,   (uint32_t)0)->asInt16(0, 1) << 2) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_WLAN_EN.syntax,     (uint32_t)0)->asInt16(0, 1) << 3) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_ROUTER_EN.syntax,   (uint32_t)0)->asInt16(0, 1) << 4) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_PHONE_EN.syntax,    (uint32_t)0)->asInt16(0, 1) << 5) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_DOCSIS_EN.syntax,   (uint32_t)0)->asInt16(0, 1) << 6) |
        (m_params.findParameter (PAR_LLDP_SYSCAP_STATION_EN.syntax,  (uint32_t)1)->asInt16(0, 1) << 7);
    m_packet.addSystemCapabilities (c, e);
}

void cLldpParser::managementAddress ()
{
    cParameter* optionalPar;
    size_t len;
    uint8_t ifNumberSubtype = m_params.findParameter (PAR_LLDP_IF_NUMBER_T.syntax, (uint32_t)1)->asInt8 ();
    uint32_t ifNumber       = m_params.findParameter (PAR_LLDP_IF_NUMBER.syntax, (uint32_t)0)->asInt32 ();
    const uint8_t* mgtOid = nullptr;
    uint8_t mgtOidLen = 0;

    optionalPar = m_params.findParameter (PAR_LLDP_MGT_OID.syntax, true);
    if (optionalPar)
    {
        mgtOid = optionalPar->asStream (len, 128);
        mgtOidLen = (uint8_t)len;
    }

    optionalPar = m_params.findParameter (PAR_LLDP_MGT_ADDR_T.syntax, true);
    if (optionalPar)
    {
        // User defined subtype. Management Address MUST be provided too
        const uint8_t* mgtAddr = m_params.findParameter (PAR_LLDP_MGT_ADDR.syntax)->asStream (len, 31);
        uint8_t subtype = optionalPar->asInt8 ();
        m_packet.addManagementAddress (subtype, mgtAddr, (uint8_t)len, ifNumberSubtype, ifNumber, mgtOid, mgtOidLen);
    }
    else
    {
        // no subtpye provided. check if user provided a management address
        optionalPar = m_params.findParameter (PAR_LLDP_MGT_ADDR.syntax, true);
        if (optionalPar)
        {
            // we accept IPv4, IPv6, MAC and bytestream.
            // try to find out the type the user provided
            try
            {
                m_packet.addManagementAddress (optionalPar->asIPv4(), ifNumberSubtype, ifNumber, mgtOid, mgtOidLen);
            }
            catch (const FormatException&)
            {
                try
                {
                    m_packet.addManagementAddress (optionalPar->asIPv6(), ifNumberSubtype, ifNumber, mgtOid, mgtOidLen);
                }
                catch (const FormatException&)
                {
                    m_packet.addManagementAddress (optionalPar->asMac(), ifNumberSubtype, ifNumber, mgtOid, mgtOidLen);
                }
            }
        }
        else
        {
            // TODO: How do we handle this case? We could use our own IP as management address.
            //       But this would mean this TLV will always be created by default

        }
    }
}

void cLldpParser::portDescription ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PORT_DESC.syntax, true);
    if (optionalPar)
    {
        size_t len;
        const uint8_t* portDescr = optionalPar->asStream (len, 255);
        m_packet.addPortDescription (portDescr, (uint8_t)len);
    }
}

void cLldpParser::systemName ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_SYSNAME.syntax, true);
    if (optionalPar)
    {
        size_t len;
        const uint8_t* sysName = optionalPar->asStream (len, 255);
        m_packet.addSystemName (sysName, (uint8_t)len);
    }
}

void cLldpParser::systemDescription ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_SYSDESC.syntax, true);
    if (optionalPar)
    {
        size_t len;
        const uint8_t* sysDescr = optionalPar->asStream (len, 255);
        m_packet.addSystemName (sysDescr, (uint8_t)len);
    }
}

void cLldpParser::portVID ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PVID.syntax, true);
    if (optionalPar)
    {
        m_packet.addPortVID (optionalPar->asInt16());
    }
}

void cLldpParser::portProtocolVID ()
{
    cParameter* ppvidPar = nullptr;
    // loop throught all PAR_LLDP_PPVID / PAR_LLDP_PPVID_* tuples
    while ((ppvidPar = m_params.findParameter(ppvidPar, nullptr, PAR_LLDP_PPVID.syntax, true)) != nullptr)
    {
        uint16_t vid = ppvidPar->asInt16 ();
        bool supported = !!m_params.findParameter (ppvidPar, PAR_LLDP_PPVID.syntax, PAR_LLDP_PPVID_SUP.syntax, (uint32_t)0)->asInt8 (0, 1);
        bool enabled   = !!m_params.findParameter (ppvidPar, PAR_LLDP_PPVID.syntax, PAR_LLDP_PPVID_EN.syntax, (uint32_t)0)->asInt8 (0, 1);

        m_packet.addProtocolVID (vid, supported, enabled);
    }
}

void cLldpParser::vlanName ()
{
    cParameter* optionalPar = nullptr;
    size_t len;
    //  loop through all PAR_LLDP_VLAN_NAME_VID / PAR_LLDP_VLAN_NAME pairs
    while ((optionalPar = m_params.findParameter(optionalPar, nullptr, PAR_LLDP_VLAN_NAME_VID.syntax, true)) != nullptr)
    {
        uint16_t vid = optionalPar->asInt16 ();
        const uint8_t* name = m_params.findParameter (optionalPar, PAR_LLDP_VLAN_NAME_VID.syntax, PAR_LLDP_VLAN_NAME.syntax)->asStream (len, 32);
        m_packet.addVlanName (vid, name, (uint8_t)len);
    }
}

void cLldpParser::protocolIdentity ()
{
    cParameter* optionalPar = nullptr;
    size_t len;
    //  loop through all PAR_LLDP_PROTO_ID
    while ((optionalPar = m_params.findParameter(optionalPar, nullptr, PAR_LLDP_PROTO_ID.syntax, true)) != nullptr)
    {
        const uint8_t* protocol = optionalPar->asStream (len, 255);
        m_packet.addProtocolIdentity (protocol, (uint8_t)len);
    }
}

void cLldpParser::vidUsageDigest ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_VID_USAGE_DIGEST.syntax, true);
    if (optionalPar)
    {
        uint32_t digest = optionalPar->asInt32 ();
        m_packet.addVIDUsageDigest (digest);
    }
}

void cLldpParser::managementVID ()
{
    cParameter* mgtVidPar = nullptr;
    // loop throught all PAR_LLDP_MGT_VID
    while ((mgtVidPar = m_params.findParameter(mgtVidPar, nullptr, PAR_LLDP_MGT_VID.syntax, true)) != nullptr)
    {
        uint16_t vid = mgtVidPar->asInt16 ();
        m_packet.addManagementVID (vid);
    }
}

void cLldpParser::linkAggregation ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_LAG_CAP.syntax, true);
    if (optionalPar)
    {
        bool cap      = !!optionalPar->asInt8 (0, 1);
        bool stat     = !!m_params.findParameter (PAR_LLDP_LAG_STATUS.syntax)->asInt8 (0, 1);
        uint8_t  type = m_params.findParameter (PAR_LLDP_LAG_PORT_TYPE.syntax)->asInt8 (0, 3);
        uint32_t id   = m_params.findParameter (PAR_LLDP_LAG_PORT_ID.syntax)->asInt32 ();
        m_packet.addLinkAggregation (cap, stat, type, id);
    }
}

void cLldpParser::congestionNotification ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_CONG_NOTE_CNPV.syntax, true);
    if (optionalPar)
    {
        uint8_t cnpv = optionalPar->asInt8 ();
        uint8_t ready = m_params.findParameter (PAR_LLDP_CONG_NOTE_READY.syntax)->asInt8 ();
        m_packet.addCongestionNotification (cnpv, ready);
    }
}

void cLldpParser::etsConfiguration ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_ETS_CFG_W.syntax, true);
    if (optionalPar)
    {
        bool willing  = !!optionalPar->asInt8 (0, 1);
        bool cbs      = !!m_params.findParameter (PAR_LLDP_ETS_CFG_CBS.syntax)->asInt8 (0, 1);
        uint8_t maxTC = m_params.findParameter (PAR_LLDP_ETS_CFG_MAX_TC.syntax)->asInt8 (0, 7);
        uint32_t prio = m_params.findParameter (PAR_LLDP_ETS_CFG_PRIO.syntax)->asInt32 ();
        uint64_t bw   = m_params.findParameter (PAR_LLDP_ETS_CFG_BW.syntax)->asInt64 ();
        uint64_t tsa  = m_params.findParameter (PAR_LLDP_ETS_CFG_TSA.syntax)->asInt64 ();
        m_packet.addETSConfig (willing, cbs, maxTC, prio, bw, tsa);
    }
}

void cLldpParser::etsRecommendation ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_ETS_REC_PRIO.syntax, true);
    if (optionalPar)
    {
        uint32_t prio = optionalPar->asInt32 ();
        uint64_t bw   = m_params.findParameter (PAR_LLDP_ETS_REC_BW.syntax)->asInt64 ();
        uint64_t tsa  = m_params.findParameter (PAR_LLDP_ETS_REC_TSA.syntax)->asInt64 ();
        m_packet.addETSRecommendation (prio, bw, tsa);
    }
}

void cLldpParser::pfcControlConfig ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PFC_W.syntax, true);
    if (optionalPar)
    {
        bool willing = !!optionalPar->asInt8 (0, 1);
        bool mbc     = !!m_params.findParameter (PAR_LLDP_PFC_MBC.syntax)->asInt8 (0, 1);
        uint8_t cap  = m_params.findParameter (PAR_LLDP_PFC_CAP.syntax)->asInt8 (0, 0x0f);
        uint8_t ena  = m_params.findParameter (PAR_LLDP_PFC_ENABLE.syntax)->asInt8 ();
        m_packet.addPFCCtrlConfig (willing, mbc, cap, ena);
    }
}

void cLldpParser::applicationPriority ()
{
    std::vector<uint8_t> prio;
    std::vector<uint8_t> sel;
    std::vector<uint16_t> proto;
    cParameter* prioPar = nullptr;
    // loop throught all PAR_LLDP_APPL_PRIO / PAR_LLDP_APPL_SEL / PAR_LLDP_APPL_PROTO tuples
    while ((prioPar = m_params.findParameter(prioPar, nullptr, PAR_LLDP_APPL_PRIO.syntax, true)) != nullptr)
    {
        prio.push_back (prioPar->asInt8 (0, 7));
        sel.push_back (m_params.findParameter (prioPar, PAR_LLDP_APPL_PRIO.syntax, PAR_LLDP_APPL_SEL.syntax)->asInt8 (0, 7));
        proto.push_back (m_params.findParameter (prioPar, PAR_LLDP_APPL_PRIO.syntax, PAR_LLDP_APPL_PROTO.syntax)->asInt16 ());
    }
    if (prio.size())
        m_packet.addApplicationPriority (prio, sel, proto);
}

void cLldpParser::evb ()
{
    cParameter* optionalPar = m_params.findParameter(PAR_LLDP_EVB_BRIDGE_STATUS.syntax, true);
    if (optionalPar)
    {
        uint8_t bridgeStatus  = optionalPar->asInt8 ();
        uint8_t stationStatus = m_params.findParameter (PAR_LLDP_EVB_STATION_STATUS.syntax)->asInt8 ();
        uint8_t retries       = m_params.findParameter (PAR_LLDP_EVB_RETRIES.syntax)->asInt8 (0, 7);
        uint8_t rte           = m_params.findParameter (PAR_LLDP_EVB_RTE.syntax)->asInt8 (0, 31);
        uint8_t mode          = m_params.findParameter (PAR_LLDP_EVB_MODE.syntax)->asInt8 (0, 3);
        bool    rolRWD        = !!m_params.findParameter (PAR_LLDP_EVB_ROL_RWD.syntax)->asInt8 (0, 1);
        uint8_t rwd           = m_params.findParameter (PAR_LLDP_EVB_RWD.syntax)->asInt8 (0, 31);
        bool    rolRKA        = !!m_params.findParameter (PAR_LLDP_EVB_ROL_RKA.syntax)->asInt8 (0, 1);
        uint8_t rka           = m_params.findParameter (PAR_LLDP_EVB_RKA.syntax)->asInt8 (0, 31);

        m_packet.addEVB (bridgeStatus, stationStatus, retries, rte, mode, rolRWD, rwd, rolRKA, rka);
    }
}

void cLldpParser::cdcb ()
{
    cParameter* optionalPar = m_params.findParameter(PAR_LLDP_CDCP_ROLE.syntax, true);
    if (optionalPar)
    {
        bool role  = !!optionalPar->asInt8 (0, 1);
        bool scomp = !!m_params.findParameter (PAR_LLDP_CDCP_SCOMP.syntax)->asInt8 (0, 1);
        uint16_t chcap =  m_params.findParameter (PAR_LLDP_CDCP_CHN_CAP.syntax)->asInt16 (0, 4095);

        std::vector<std::pair <uint16_t, uint16_t>> scid_svid;
        //  loop through all PAR_LLDP_CDCP_SCID / PAR_LLDP_CDCP_SVID pairs
        while ((optionalPar = m_params.findParameter(optionalPar, nullptr, PAR_LLDP_CDCP_SCID.syntax, true)) != nullptr)
        {
            uint16_t scid = optionalPar->asInt16 (0, 4095);
            uint16_t svid = m_params.findParameter (optionalPar, PAR_LLDP_CDCP_SCID.syntax, PAR_LLDP_CDCP_SVID.syntax)->asInt16 (0, 4095);
            scid_svid.push_back (std::pair<uint16_t, uint16_t>(scid, svid));
        }
        m_packet.addCDCP (role, scomp, chcap, scid_svid);
    }
}

void cLldpParser::applicationVLAN ()
{
    std::vector<uint16_t> vid;
    std::vector<uint8_t> sel;
    std::vector<uint16_t> proto;
    cParameter* prioPar = nullptr;
    // loop throught all PAR_LLDP_APPL_VLAN_VID / PAR_LLDP_APPL_VLAN_SEL / PAR_LLDP_APPL_VLAN_PROTO tuples
    while ((prioPar = m_params.findParameter(prioPar, nullptr, PAR_LLDP_APPL_VLAN_VID.syntax, true)) != nullptr)
    {
        vid.push_back (prioPar->asInt16 (0, 0x03FF));
        sel.push_back (m_params.findParameter (prioPar, PAR_LLDP_APPL_VLAN_VID.syntax, PAR_LLDP_APPL_VLAN_SEL.syntax)->asInt8 (0, 7));
        proto.push_back (m_params.findParameter (prioPar, PAR_LLDP_APPL_VLAN_VID.syntax, PAR_LLDP_APPL_VLAN_PROTO.syntax)->asInt16 ());
    }
    if (vid.size())
        m_packet.addApplicationVLAN (vid, sel, proto);
}

void cLldpParser::macPhyConfigStatus ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_MACPHY_MAU_TYPE.syntax, true);
    if (optionalPar)
    {
        bool supported   = !!m_params.findParameter (PAR_LLDP_MACPHY_ANEG_SUP.syntax, (uint32_t)0)->asInt8 (0, 1);
        bool enabled     = !!m_params.findParameter (PAR_LLDP_MACPHY_ANEG_ENA.syntax, (uint32_t)0)->asInt8 (0, 1);
        uint16_t caps    = m_params.findParameter (PAR_LLDP_MACPHY_ANEG_CAPS.syntax)->asInt16();
        uint16_t mautype = m_params.findParameter (PAR_LLDP_MACPHY_MAU_TYPE.syntax)->asInt16();

        m_packet.addMacPhyStatus (supported, enabled, caps, mautype);
    }
}

void cLldpParser::powerViaMDI ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_POE_MDI_POWER_SUP_PORT_CLASS.syntax, true);

    // if port class is provided, we expect ALL basic fields to be present
    if (optionalPar)
    {
        // MDI power support field
        bool portClassPSE    = !!optionalPar->asInt8 (0, 1);
        bool pwrSupSupported = !!m_params.findParameter (PAR_LLDP_POE_MDI_POWER_SUP_PSE_MDI_SUP.syntax)->asInt8 (0, 1);
        bool pwrSupState     = !!m_params.findParameter (PAR_LLDP_POE_MDI_POWER_SUP_PSE_MDI_ENA.syntax)->asInt8 (0, 1);
        bool pwrSupPairsCtrl = !!m_params.findParameter (PAR_LLDP_POE_MDI_POWER_SUP_PSE_PAIR_CTRL.syntax)->asInt8 (0, 1);

        uint8_t psePowerPair =   m_params.findParameter (PAR_LLDP_POE_PSE_POWER_PAIR.syntax)->asInt8 ();
        uint8_t powerClass   =   m_params.findParameter (PAR_LLDP_POE_POWER_CLASS.syntax)->asInt8 ();

        optionalPar = m_params.findParameter (PAR_LLDP_POE_DLL_POWER_TYPE.syntax, true);
        // if power type is provided, we expect ALL DLL classification extension fields to be present
        if (optionalPar)
        {
            uint8_t powerType   = optionalPar->asInt8 ();
            uint8_t powerSource = m_params.findParameter (PAR_LLDP_POE_DLL_POWER_SOURCE.syntax)->asInt8 ();
            uint8_t pd4pid      = m_params.findParameter (PAR_LLDP_POE_DLL_PD_4PID.syntax)->asInt8 ();
            uint8_t powerPrio   = m_params.findParameter (PAR_LLDP_POE_DLL_POWER_PRIO.syntax)->asInt8 ();

            double pdRequestedPower  = m_params.findParameter (PAR_LLDP_POE_DLL_PD_REQ_POWER.syntax)->asDouble (0, 99.0);
            double pseRequestedPower = m_params.findParameter (PAR_LLDP_POE_DLL_PD_ALLOC_POWER.syntax)->asDouble (0, 99.0);

            m_packet.addDllExtPowerViaMDI (portClassPSE, pwrSupSupported, pwrSupState, pwrSupPairsCtrl, psePowerPair, powerClass,
                powerType, powerSource, pd4pid, powerPrio, pdRequestedPower, pseRequestedPower);

            // TODO Type 3 and Type 4 extension
        }
        else
        {
            m_packet.addBasicPowerViaMDI (portClassPSE, pwrSupSupported, pwrSupState, pwrSupPairsCtrl, psePowerPair, powerClass);
        }
    }
}

void cLldpParser::maxFrameSize ()
{
    uint16_t frameSize =  m_params.findParameter (PAR_LLDP_MAX_FRAME_SIZE.syntax, (uint32_t)0)->asInt16 ();
    if (frameSize)
        m_packet.addMaxFrameSize (frameSize);
}

void cLldpParser::eee ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_EEE_TX_TW.syntax, true);
    if (optionalPar)
    {
        uint16_t tx = optionalPar->asInt16 ();
        uint16_t rx = m_params.findParameter (PAR_LLDP_EEE_RX_TW.syntax)->asInt16 ();
        uint16_t fb = m_params.findParameter (PAR_LLDP_EEE_FB_RX_TW.syntax)->asInt16 ();
        uint16_t echoTx = m_params.findParameter (PAR_LLDP_EEE_ECHO_TX_TW.syntax)->asInt16 ();
        uint16_t echoRx = m_params.findParameter (PAR_LLDP_EEE_ECHO_RX_TW.syntax)->asInt16 ();
        m_packet.addEEE (tx, rx, fb, echoTx, echoRx);
    }
}

void cLldpParser::eeeFastWake ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_EEE_FW_TX.syntax, true);
    if (optionalPar)
    {
        bool tx     = !!optionalPar->asInt8 (0, 1);
        bool rx     = !!m_params.findParameter (PAR_LLDP_EEE_FW_RX.syntax)->asInt8 (0, 1);
        bool echoTx = !!m_params.findParameter (PAR_LLDP_EEE_FW_ECHO_TX.syntax)->asInt8 (0, 1);
        bool echoRx = !!m_params.findParameter (PAR_LLDP_EEE_FW_ECHO_RX.syntax)->asInt8 (0, 1);
        m_packet.addEEEFastWake (tx, rx, echoTx, echoRx);
    }
}

void cLldpParser::pnDelay ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PN_DELAY_PORT_RX_LOC.syntax, true);
    if (optionalPar)
    {
        m_packet.addPnDelay (optionalPar->asInt32(),
            m_params.findParameter (PAR_LLDP_PN_DELAY_PORT_RX_REM.syntax)->asInt32 (),
            m_params.findParameter (PAR_LLDP_PN_DELAY_PORT_TX_LOC.syntax)->asInt32 (),
            m_params.findParameter (PAR_LLDP_PN_DELAY_PORT_TX_REM.syntax)->asInt32 (),
            m_params.findParameter (PAR_LLDP_PN_DELAY_LINE.syntax)->asInt32 ());
    }
}

void cLldpParser::pnPortStatus ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PN_RTC3_STATE.syntax, true);
    if (optionalPar)
    {
        uint8_t rtc3State = optionalPar->asInt8 (0, 7);
        bool frag         = !!m_params.findParameter (PAR_LLDP_PN_RTC3_FRAG.syntax, uint32_t(0))->asInt8 (0, 1);
        bool shortPreamp  = !!m_params.findParameter (PAR_LLDP_PN_RTC3_PREAMP.syntax, uint32_t(0))->asInt8 (0, 1);
        bool optimized    = !!m_params.findParameter (PAR_LLDP_PN_RTC3_OPTIMIZED.syntax, uint32_t(0))->asInt8 (0, 1);
        uint16_t rtc2State =  m_params.findParameter (PAR_LLDP_PN_RTC2_STATE.syntax, uint32_t(0))->asInt16 (0, 3);

        m_packet.addPnPortStatus (rtc2State, rtc3State, frag, shortPreamp, optimized);
    }
}

void cLldpParser::pnAlias ()
{
    cParameter* aliasPar = m_params.findParameter (PAR_LLDP_PN_ALIAS.syntax, true);
    if (aliasPar)
    {
        size_t len;
        const uint8_t* alias = aliasPar->asStream (len, 255);
        m_packet.addPnAlias (alias, (uint8_t)len);
    }
}

void cLldpParser::pnMrpPortState ()
{
    uint16_t portState = m_params.findParameter (PAR_LLDP_PN_MRP_MRRT_STATE.syntax, uint32_t(0))->asInt16 (0, 3);
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PN_MRP_DOMAIN.syntax, true);
    size_t len;

    // if user provided the MRP domain, we calculate the domainUUID
    // if not, the domainUUID has to be provided by the user
    if (optionalPar)
    {
        const uint8_t* domain = optionalPar->asStream (len);
        cMD5 md5;
        cUUID uuid = cUUID::fromMD5 (md5.calc (domain, len));
        m_packet.addPnMrpPortStatus (uuid.asArray(), portState);

    }
    else
    {
        optionalPar = m_params.findParameter (PAR_LLDP_PN_MRP_DOMAIN_UUID.syntax, true);
        if (optionalPar)
        {
            cUUID uuid = optionalPar->asUUID ();
            m_packet.addPnMrpPortStatus (uuid.asArray(), portState);
        }
    }
}

void cLldpParser::pnChassisMac ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PN_CHASSIS_MAC.syntax, true);
    if (optionalPar)
    {
        m_packet.addPnChassisMac (optionalPar->asMac ());
    }
}

void cLldpParser::pnPTCPStatus ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PN_PTCP_MAST_SRC_MAC.syntax, true);
    if (optionalPar)
    {
        cMacAddress mac = optionalPar->asMac ();

        optionalPar = m_params.findParameter (PAR_LLDP_PN_PTCP_DOMAIN_UUID.syntax, true);
        cUUID uuidDomain = optionalPar ? optionalPar->asUUID () : cUUID::fromZero ();
        optionalPar = m_params.findParameter (PAR_LLDP_PN_PTCP_IRDATA_UUID.syntax, true);
        cUUID uuidIRDATA = optionalPar ? optionalPar->asUUID () : cUUID::fromZero ();

        uint32_t period = m_params.findParameter (PAR_LLDP_PN_PTCP_PERIOD_LEN.syntax, uint32_t (0))->asInt32 (0, 0x7fffffff);
        uint32_t redOrange = m_params.findParameter (PAR_LLDP_PN_PTCP_RED_ORANGE.syntax, uint32_t (0))->asInt32 (0, 0x7fffffff);
        uint32_t orange = m_params.findParameter (PAR_LLDP_PN_PTCP_ORANGE.syntax, uint32_t (0))->asInt32 (0, 0x7fffffff);
        uint32_t green = m_params.findParameter (PAR_LLDP_PN_PTCP_GREEN.syntax, uint32_t (0))->asInt32 (0, 0x7fffffff);

        m_packet.addPnPtcpStatus (mac, uuidDomain.asArray(), uuidIRDATA.asArray(), 
            period, !!period,
            redOrange, !!redOrange,
            orange, !!orange,
            green, !!green);
    }
}

void cLldpParser::pnMAUTypeExtension ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PN_MAU_TYPE_EXT.syntax, true);
    if (optionalPar)
    {
        m_packet.addPnMauTypeExtension (optionalPar->asInt16 ());
    }
}

void cLldpParser::pnMrpIcPortStatus ()
{
    cParameter* optionalPar = m_params.findParameter (PAR_LLDP_PN_MRP_IC_DOMAIN_ID.syntax, true);
    if (optionalPar)
    {
        uint16_t id = optionalPar->asInt16 ();
        uint16_t role = m_params.findParameter (PAR_LLDP_PN_MRP_IC_ROLE.syntax, uint32_t (0))->asInt16 ();
        uint16_t pos = m_params.findParameter (PAR_LLDP_PN_MRP_IC_MIC_POS.syntax, uint32_t (0))->asInt16 ();
        m_packet.addPnMrpInterconnectPortStatus (id, role, pos);
    }
}

void cLldpParser::allRawTLVs ()
{
    cParameter* optionalPar = nullptr;
    size_t len;
    //  loop through all PAR_LLDP_TLV_TYPE / PAR_LLDP_TLV_VALUE pairs
    while ((optionalPar = m_params.findParameter(optionalPar, nullptr, PAR_LLDP_TLV_TYPE.syntax, true)) != nullptr)
    {
        uint8_t type = optionalPar->asInt8 (0, 127);
        const uint8_t* value = m_params.findParameter (optionalPar, PAR_LLDP_TLV_TYPE.syntax, PAR_LLDP_TLV_VALUE.syntax)->asStream (len, 511);
        m_packet.addRawTLV (type, value, (uint16_t)len);
    }
}

void cLldpParser::allOidTLVs ()
{
    cParameter* optionalPar = nullptr;
    size_t len;
    //  loop through all PAR_LLDP_OUI_TLV_OUI / PAR_LLDP_OUI_TLV_TYPE / PAR_LLDP_OUI_TLV_VALUE tuples
    while ((optionalPar = m_params.findParameter(optionalPar, nullptr, PAR_LLDP_OUI_TLV_OUI.syntax, true)) != nullptr)
    {
        const uint8_t*   oui = optionalPar->asStream (len, 3);
        uint8_t      subtype = m_params.findParameter (optionalPar, PAR_LLDP_OUI_TLV_OUI.syntax, PAR_LLDP_OUI_TLV_TYPE.syntax)->asInt8 ();
        const uint8_t* value = m_params.findParameter (optionalPar, PAR_LLDP_OUI_TLV_OUI.syntax, PAR_LLDP_OUI_TLV_VALUE.syntax)->asStream (len, 507);
        m_packet.addOuiTLV (oui, subtype, value, (uint16_t)len);
    }
}
