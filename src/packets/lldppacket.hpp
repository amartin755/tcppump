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


#ifndef LLDP_PACKET_H_
#define LLDP_PACKET_H_

#include <cstdint>
#include <vector>
#include <utility>

#include "ethernetpacket.hpp"
#include "ipaddress.hpp"
#include "bytearray.hpp"


class cLldpPacket : public cEthernetPacket
{
public:
    cLldpPacket ();
    ~cLldpPacket ();

    void addChassisID (const cMacAddress& mac);
    void addChassisID (const cIPv4& ip);
    void addChassisID (const cIPv6& ip);
    void addChassisID (uint8_t subtype, const uint8_t* id, uint8_t len);
    void addPortID (const cMacAddress& mac);
    void addPortID (const cIPv4& ip);
    void addPortID (const cIPv6& ip);
    void addPortID (uint8_t subtype, const uint8_t* id, uint8_t len);
    void addTTL (uint16_t ttl);
    void addPortDescription (const uint8_t* portdescr, uint8_t len);
    void addSystemName (const uint8_t* sysname, uint8_t len);
    void addSystemDescription (const uint8_t* sysdescr, uint8_t len);
    void addSystemCapabilities (uint16_t system, uint16_t enabled);
    void addManagementAddress (const cMacAddress& mgtAddr,
        uint8_t ifNbSubtype, uint32_t ifNumber, const uint8_t* oid, uint8_t oidLen);
    void addManagementAddress (const cIPv4& mgtAddr,
        uint8_t ifNbSubtype, uint32_t ifNumber, const uint8_t* oid, uint8_t oidLen);
    void addManagementAddress (const cIPv6& mgtAddr,
        uint8_t ifNbSubtype, uint32_t ifNumber, const uint8_t* oid, uint8_t oidLen);
    void addManagementAddress (uint8_t mgtAddrSubtype, const uint8_t* mgtAddr, uint8_t mgtAddrLen,
        uint8_t ifNbSubtype, uint32_t ifNumber, const uint8_t* oid, uint8_t oidLen);
    void addPortVID (uint16_t pvid);
    void addProtocolVID (uint16_t ppvid, bool supported, bool enabled);
    void addProtocolIdentity (const uint8_t* protocol, uint8_t protocolLength);
    void addVlanName (uint16_t vid, const uint8_t* name, uint8_t nameLength);
    void addVIDUsageDigest (uint32_t digest);
    void addManagementVID (uint16_t vid);
    void addLinkAggregation (bool capability, bool status, uint8_t portType, uint32_t portID);
    void addCongestionNotification (uint8_t cnpv, uint8_t ready);
    void addETSConfig (bool willing, bool cbs, uint8_t maxTCs,
        uint32_t prioTable, uint64_t tcBandwidthTabel, uint64_t tsaAssignmentTable);
    void addETSRecommendation (uint32_t prioTable, uint64_t tcBandwidthTabel, uint64_t tsaAssignmentTable);
    void addPFCCtrlConfig (bool willing, bool mbc, uint8_t pfcCap, uint8_t pfcEnable);
    void addApplicationPriority (const std::vector<uint8_t>& prio,
        const std::vector<uint8_t>&sel, const std::vector<uint16_t>& proto);
    void addEVB (uint8_t evbBridgeStatus, uint8_t evbStationStatus,
        uint8_t r, uint8_t rte, uint8_t evb, bool rolRwd, uint8_t rwd, bool rolRka, uint8_t rka);
    void addCDCP (bool role, bool SComp, uint16_t chnCap, const std::vector<std::pair <uint16_t, uint16_t>>&scid_svid);
    void addApplicationVLAN (const std::vector<uint16_t>& vid,
        const std::vector<uint8_t>&sel, const std::vector<uint16_t>& proto);

    void addMacPhyStatus (bool autonegSup, bool autonegStatus, uint16_t autnegAdvCap, uint16_t mautype);
    void addBasicPowerViaMDI (bool pwrSupPortClass /*false = PD, true = PSE*/, bool pwrSupSupported, bool pwrSupState, bool pwrSupPairsCtrl,
        uint8_t psePowerPair, uint8_t powerClass);
    void addDllExtPowerViaMDI (bool pwrSupPortClass /*false = PD, true = PSE*/, bool pwrSupSupported, bool pwrSupState, bool pwrSupPairsCtrl,
        uint8_t psePowerPair, uint8_t powerClass,
        uint8_t powerType, uint8_t powerSource, uint8_t pd4pid, uint8_t powerPrio, double pdRequestedPower, double pseRequestedPower);
    void addMaxFrameSize (uint16_t size);
    void addEEE (uint16_t txTw, uint16_t rxTw, uint16_t fbTw, uint16_t echoTxTw, uint16_t echoRxTw);
    void addEEEFastWake (uint8_t tx, uint8_t rx, uint8_t echoTx, uint8_t echoRx);

    void addPnDelay (uint32_t portRxDelayLocal, uint32_t portRxDelayRemote,
        uint32_t portTxDelayLocal, uint32_t portTxDelayRemote, uint32_t cableDelay);
    void addPnPortStatus (uint16_t rtc2PortStatus, uint8_t rtc3State,
        bool rtc3Frag, bool rtc3shortPreamp, bool rtc3optimized);
    void addPnAlias (const uint8_t* aliasName, uint8_t len);
    void addPnMrpPortStatus (const uint8_t* domainUUID, uint16_t mrrtPortState);
    void addPnChassisMac (const cMacAddress& chassisMac);
    void addPnPtcpStatus (const cMacAddress& masterSourceMac, const uint8_t* ptcpSubdomainUUID,
        const uint8_t* irdataUUID, uint32_t lengthOfPeriod, bool lengthOfPeriodValid,
        uint32_t redOrangePeriodBegin, bool redOrangePeriodBeginValid,
        uint32_t orangePeriodBegin, bool orangePeriodBeginValid,
        uint32_t greenPeriodBegin, bool greenPeriodBeginValid);
    void addPnMauTypeExtension (uint16_t mauTypeExtension);
    void addPnMrpInterconnectPortStatus (uint16_t domainID, uint16_t role, uint16_t position);
    void addPnNmeDomainUUID (const uint8_t* nmeSubdomainUUID);
    void addPnNmeManagementAddr (const cMacAddress& mgtAddr)
    {
        addPnNmeManagementAddr (6 /*all802*/, (uint8_t*)mgtAddr.get(), (uint8_t)mgtAddr.size());
    }
    void addPnNmeManagementAddr (const cIPv4& mgtAddr)
    {
        addPnNmeManagementAddr (1 /*ipV4*/, mgtAddr.getAsArray(), 4);
    }
    void addPnNmeManagementAddr (const cIPv6& mgtAddr)
    {
        addPnNmeManagementAddr (2 /*ipV6*/, (uint8_t*)mgtAddr.getAsArray(), 16);
    }
    void addPnNmeManagementAddr (uint8_t mgtAddrSubtype, const uint8_t* mgtAddr, uint8_t mgtAddrLen);
    void addPnNmeNameUUID (const uint8_t* nmeNameUUID);
    void addPnNmeParameterUUID (const uint8_t* nmeParameterUUID);
    void addPn1AsWorkingClock (uint16_t timeDomainNumber, const uint8_t* clockIdentity);
    void addPn1AsGlobalTime (uint16_t timeDomainNumber, const uint8_t* clockIdentity);

    void addRawTLV (uint8_t type, const uint8_t* value, uint16_t valLen /*9 bits are allowed -> 0 - 511 octets*/);
    void addOuiTLV (const uint8_t* oui /*3 bytes*/, uint8_t subtype, const uint8_t* value, uint16_t valLen /* 0 - 507 octets*/);
    void addEndTLV ();

    void compile (bool withEndTLV = true);

    enum TLV_TYPE : uint8_t
    {
        END = 0,
        CHASSIS_ID = 1,
        PORT_ID = 2,
        TTL = 3,
        PORT_DESCR = 4,
        SYSNAME = 5,
        SYSDESC = 6,
        SYSCAP = 7,
        MGT_ADDR = 8,
        OUI = 127
    };

    enum TLV_SUBTYPE_802_1 : uint8_t
    {
        PVID = 1,
        PROTO_VID = 2,
        VLAN_NAME = 3,
        PROTO_IDENTITY = 4,
        VID_USAGE_DIGEST = 5,
        MGT_VID = 6,
        LINK_AGGR = 7,
        CONGESTION_NOTIFICATION = 8,
        ETS_CONFIG = 9,
        ETS_RECOMM = 10,
        PFC_CONFIG = 11,
        APP_PRIO = 12,
        EVB = 13,
        CDCP = 14,
        APP_VLAN = 16
    };

    enum TLV_SUBTYPE_802_3 : uint8_t
    {
        MAC_PHY = 1,
        POWER_VIA_MDI = 2,
        LINK_AGGR_DEPR = 3, // deprecated
        MAX_FRAME_SIZE = 4,
        ENERGY_EFFICIENT_ETH = 5,
        EEE_FAST_WAKE = 6,
        ADD_ETH_CAPS = 7,
        POWER_VIA_MDI_MEAS = 8
    };

    enum TLV_SUBTYPE_PN : uint8_t
    {
        PN_DELAY = 1,
        PN_PORT_STATUS = 2,
        PN_ALIAS = 3,
        PN_MRP_PORT_STATUS = 4,
        PN_CHASSIS_MAC = 5,
        PN_PTCP_STATUS = 6,
        PN_MAUTYPE_EXT = 7,
        PN_MRPIC_PORT_STATUS = 8,
        PN_NME_DOMAIN_UUID = 9,
        PN_NME_MGT_ADDR = 10,
        PN_NME_NAME_UUID = 11,
        PN_NME_PAR_UUID = 12,
        PN_AS_WORKING_CLOCK = 13,
        PN_AS_GLOBAL_TIME = 14,
    };

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    cFixedByteArray* preparePowerViaMDI (size_t tlvLen,
        bool pwrSupPortClass, bool pwrSupSupported, bool pwrSupState, bool pwrSupPairsCtrl,
        uint8_t psePowerPair, uint8_t powerClass) const;
    uint16_t doubleToPowerValue (double power) const
    {
        double units = power / 0.1;
        return units > 65535 ? 65535 : (uint16_t)units;
    }
    std::vector< std::pair<uint8_t, const cFixedByteArray*> > m_tlvs;
};


#endif /* LLDP_PACKET_H_ */
