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
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "lldppacket.hpp"
#include "bug.hpp"
#include "endian.h"


static const uint8_t OID_802_1[3] = {0x00,0x80,0xC2};
static const uint8_t OID_802_3[3] = {0x00,0x12,0x0F};
// for future use
// avoid clang warning  static const uint8_t OID_TIA[3]   = {0x00,0x12,0xBB};
static const uint8_t OID_PNO[3]   = {0x00,0x0E,0xCF};


cLldpPacket::cLldpPacket ()
{
    m_tlvs.reserve (16); // set intial capacity to a value that fits for most use-cases
}

cLldpPacket::~cLldpPacket ()
{
    for (auto& tlv : m_tlvs)
    {
        delete tlv.second;
    }
}

void cLldpPacket::addChassisID (const cMacAddress& mac)
{
    addChassisID (4, (const uint8_t*)mac.get (), (uint8_t)mac.size());
}

void cLldpPacket::addChassisID (const cIPv4& ip)
{
    uint8_t chassisID[sizeof(ip) + 1];
    chassisID[0] = 1; // ipV4(1)
    std::memcpy (&chassisID[1], ip.getAsArray(), sizeof (ip));

    addChassisID (5, chassisID, sizeof (chassisID));
}

void cLldpPacket::addChassisID (const cIPv6& ip)
{
    uint8_t chassisID[sizeof(ip) + 1];
    chassisID[0] = 2; // ipV6(2)
    std::memcpy (&chassisID[1], ip.getAsArray(), sizeof (ip));

    addChassisID (5, chassisID, sizeof (chassisID));
}

void cLldpPacket::addChassisID (uint8_t subtype, const uint8_t* id, uint8_t len)
{
    cFixedByteArray* value = new cFixedByteArray (len + 1);
    *value << subtype;
    value->append (id, len);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (CHASSIS_ID, value));
}

void cLldpPacket::addPortID (const cMacAddress& mac)
{
    addPortID (3, (const uint8_t*)mac.get (), (uint8_t)mac.size());
}

void cLldpPacket::addPortID (const cIPv4& ip)
{
    uint8_t portID[sizeof(ip) + 1];
    portID[0] = 1; // ipV4(1)
    std::memcpy (&portID[1], ip.getAsArray(), sizeof (ip));

    addPortID (4, portID, sizeof (portID));
}

void cLldpPacket::addPortID (const cIPv6& ip)
{
    uint8_t portID[sizeof(ip) + 1];
    portID[0] = 1; // ipV4(1)
    std::memcpy (&portID[1], ip.getAsArray(), sizeof (ip));

    addPortID (4, portID, sizeof (portID));
}

void cLldpPacket::addPortID (uint8_t subtype, const uint8_t* id, uint8_t len)
{
    cFixedByteArray* value = new cFixedByteArray (len + 1);
    *value << subtype;
    value->append (id, len);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (PORT_ID, value));
}

void cLldpPacket::addTTL (uint16_t ttl)
{
    cFixedByteArray* value = new cFixedByteArray (sizeof (ttl));
    *value << toBE16(ttl);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (TTL, value));
}

void cLldpPacket::addPortDescription (const uint8_t* portdescr, uint8_t len)
{
    cFixedByteArray* value = new cFixedByteArray (len);
    value->append (portdescr, len);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (PORT_DESCR, value));
}

void cLldpPacket::addSystemName (const uint8_t* sysname, uint8_t len)
{
    cFixedByteArray* value = new cFixedByteArray (len);
    value->append (sysname, len);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (SYSNAME, value));
}

void cLldpPacket::addSystemDescription (const uint8_t* sysdescr, uint8_t len)
{
    cFixedByteArray* value = new cFixedByteArray (len);
    value->append (sysdescr, len);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (SYSDESC, value));
}

void cLldpPacket::addSystemCapabilities (uint16_t system, uint16_t enabled)
{
    cFixedByteArray* value = new cFixedByteArray (sizeof (system) *2);
    *value << toBE16(system) << toBE16(enabled);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (SYSCAP, value));
}

void cLldpPacket::addManagementAddress (const cMacAddress& mgtAddr,
    uint8_t ifNbSubtype, uint32_t ifNumber, const uint8_t* oid, uint8_t oidLen)
{
    addManagementAddress (6 /*all802*/, (uint8_t*)mgtAddr.get(), (uint8_t)mgtAddr.size(),
        ifNbSubtype, ifNumber, oid, oidLen);
}

void cLldpPacket::addManagementAddress (const cIPv4& mgtAddr,
    uint8_t ifNbSubtype, uint32_t ifNumber, const uint8_t* oid, uint8_t oidLen)
{
    addManagementAddress (1 /*ipV4*/, mgtAddr.getAsArray(), 4,
        ifNbSubtype, ifNumber, oid, oidLen);
}

void cLldpPacket::addManagementAddress (const cIPv6& mgtAddr,
    uint8_t ifNbSubtype, uint32_t ifNumber, const uint8_t* oid, uint8_t oidLen)
{
    addManagementAddress (2 /*ipV6*/, (uint8_t*)mgtAddr.getAsArray(), 16,
        ifNbSubtype, ifNumber, oid, oidLen);
}

void cLldpPacket::addManagementAddress (uint8_t mgtAddrSubtype, const uint8_t* mgtAddr, uint8_t mgtAddrLen,
    uint8_t ifNbSubtype, uint32_t ifNumber, const uint8_t* oid, uint8_t oidLen)
{
    const size_t tlvLen = 1+1+mgtAddrLen + 1+sizeof (ifNumber) + 1+oidLen;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    *value << uint8_t(mgtAddrLen+1) << mgtAddrSubtype;
    value->append (mgtAddr, mgtAddrLen);
    *value << ifNbSubtype << toBE32(ifNumber) << oidLen;
    if (oidLen)
        value->append (oid, oidLen);

    assert (value->capacity() == tlvLen && value->size() == tlvLen);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (MGT_ADDR, value));
}

void cLldpPacket::addPortVID (uint16_t pvid)
{
    const size_t tlvLen = sizeof (pvid) + sizeof (OID_802_1) + 1;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::PVID << toBE16(pvid);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addProtocolVID (uint16_t ppvid, bool supported, bool enabled)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1 + 1 + sizeof (ppvid);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::PROTO_VID;
    *value << (uint8_t)((supported ? 2 : 0) | (enabled ? 4 : 0));
    *value << toBE16 (ppvid);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addVlanName (uint16_t vid, const uint8_t* name, uint8_t nameLength)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1 + sizeof (vid) + sizeof (nameLength) + nameLength;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::VLAN_NAME << toBE16 (vid) << nameLength;
    value->append (name, nameLength);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addProtocolIdentity (const uint8_t* protocol, uint8_t protocolLength)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1 + sizeof (protocolLength) + protocolLength;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::PROTO_IDENTITY << protocolLength;
    value->append (protocol, protocolLength);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addVIDUsageDigest (uint32_t digest)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1 + sizeof (digest);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::VID_USAGE_DIGEST << toBE32(digest);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addManagementVID (uint16_t vid)
{
    const size_t tlvLen = sizeof (vid) + sizeof (OID_802_1) + 1;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::MGT_VID << toBE16(vid);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addLinkAggregation (bool capability, bool status, uint8_t portType, uint32_t portID)
{
    const size_t tlvLen = sizeof (status) + sizeof (portID) + sizeof (OID_802_1) + 1;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::LINK_AGGR;
    *value << (uint8_t)((capability ? 1U : 0U) | (status ? 2U : 0U) | ((portType & 3) << 2));
    *value << toBE32(portID);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addCongestionNotification (uint8_t cnpv, uint8_t ready)
{
    const size_t tlvLen = sizeof (cnpv) + sizeof (ready) + sizeof (OID_802_1) + 1;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::CONGESTION_NOTIFICATION << cnpv << ready;

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addETSConfig (bool willing, bool cbs, uint8_t maxTCs,
    uint32_t prioTable, uint64_t tcBandwidthTabel, uint64_t tsaAssignmentTable)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1
        + 1 /*willing, cbs, reserved, maxTCs*/
        + sizeof (prioTable) + sizeof (tcBandwidthTabel) + sizeof (tsaAssignmentTable);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::ETS_CONFIG;
    *value << (uint8_t)((willing ? 0x80 : 0) | (cbs ? 0x40 : 0) | (maxTCs & 7));
    *value << toBE32 (prioTable) << toBE64 (tcBandwidthTabel) << toBE64 (tsaAssignmentTable);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addETSRecommendation (uint32_t prioTable, uint64_t tcBandwidthTabel, uint64_t tsaAssignmentTable)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1
        + 1 /*reserved*/
        + sizeof (prioTable) + sizeof (tcBandwidthTabel) + sizeof (tsaAssignmentTable);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::ETS_RECOMM;
    *value << (uint8_t)0;
    *value << toBE32 (prioTable) << toBE64 (tcBandwidthTabel) << toBE64 (tsaAssignmentTable);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPFCCtrlConfig (bool willing, bool mbc, uint8_t pfcCap, uint8_t pfcEnable)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1
        + 1 /*willing, mbc, reserved, pfcCap*/
        + sizeof (pfcEnable);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::PFC_CONFIG;
    *value << (uint8_t)((willing ? 0x80 : 0) | (mbc ? 0x40 : 0) | (pfcCap & 0x0f));
    *value << pfcEnable;

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addApplicationPriority (const std::vector<uint8_t>& prio,
    const std::vector<uint8_t>&sel, const std::vector<uint16_t>& proto)
{
    BUG_ON (prio.size() != sel.size() || prio.size() != proto.size());

    const size_t tlvLen = sizeof (OID_802_1) + 1 + 1 + prio.size() * 3;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::APP_PRIO;
    *value << (uint8_t)0;
    for (size_t n = 0; n < prio.size(); n++)
    {
        *value << uint8_t(((prio[n] & 7) << 5) | (sel[n] & 7));
        *value << toBE16 (proto[n]);
    }
    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addEVB (uint8_t evbBridgeStatus, uint8_t evbStationStatus,
    uint8_t r, uint8_t rte, uint8_t evb, bool rolRwd, uint8_t rwd, bool rolRka, uint8_t rka)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1 + 5;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::EVB;
    *value << evbBridgeStatus;
    *value << evbStationStatus;
    *value << uint8_t(((r & 7) << 5 ) | (rte & 0x1f));
    *value << uint8_t(((evb & 3) << 6 ) | (rolRwd ? 0x20 : 0) | (rwd & 0x1f));
    *value << uint8_t((rolRka ? 0x20 : 0) | (rka & 0x1f));

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addCDCP (bool role, bool SComp, uint16_t chnCap,
    const std::vector<std::pair <uint16_t, uint16_t>>&scid_svid)
{
    const size_t tlvLen = sizeof (OID_802_1) + 1 + 4 + scid_svid.size() * 3;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::CDCP;
    *value << uint8_t((role ? 0x80 : 0) | (SComp ? 8 : 0));
    *value << uint8_t(0);
    *value << uint8_t((chnCap >> 8) & 0xf);
    *value << uint8_t(chnCap & 0x00ff);
    for (const auto& val : scid_svid)
    {
        *value << uint8_t(val.first >> 4);
        *value << uint8_t(((val.first & 0xf) << 4)| ((val.second >> 8) & 0x0f));
        *value << uint8_t(val.second);
    }

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addApplicationVLAN (const std::vector<uint16_t>& vid,
    const std::vector<uint8_t>&sel, const std::vector<uint16_t>& proto)
{
    BUG_ON (vid.size() != sel.size() || sel.size() != proto.size());

    const size_t tlvLen = sizeof (OID_802_1) + 1 + vid.size() * 4;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_1, sizeof (OID_802_1));
    *value << (uint8_t)TLV_SUBTYPE_802_1::APP_VLAN;
    for (size_t n = 0; n < vid.size(); n++)
    {
        uint32_t vlan = ((uint32_t)toBE16(vid[n] & 0x03ff) << 20) | (uint32_t(sel[n] & 7) << 16) | toBE16 (proto[n]);
        *value << vlan;
    }
    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addMacPhyStatus (bool autonegSup, bool autonegStatus, uint16_t autnegAdvCap, uint16_t mautype)
{
    const size_t tlvLen = sizeof (OID_802_3) + 1 + 1 + sizeof (autnegAdvCap) + sizeof (mautype);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_3, sizeof (OID_802_3));
    *value << (uint8_t)TLV_SUBTYPE_802_3::MAC_PHY;
    *value << (uint8_t)((autonegSup ? 1 : 0) | (autonegStatus ? 2 : 0));
    *value << toBE16 (autnegAdvCap);
    *value << toBE16 (mautype);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

cFixedByteArray* cLldpPacket::preparePowerViaMDI (size_t tlvLen,
    bool portClassPSE, bool pwrSupSupported, bool pwrSupState, bool pwrSupPairsCtrl,
    uint8_t psePowerPair, uint8_t powerClass) const
{
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_3, sizeof (OID_802_3));
    *value << (uint8_t)TLV_SUBTYPE_802_3::POWER_VIA_MDI;
    *value << (uint8_t)(
        (portClassPSE ? 1 : 0)    |
        (pwrSupSupported ? 2 : 0) |
        (pwrSupState ? 4 : 0)     |
        (pwrSupPairsCtrl ? 8 : 0));
    *value << psePowerPair;
    *value << powerClass;
   return value;
}

void cLldpPacket::addBasicPowerViaMDI (
    bool portClassPSE, bool pwrSupSupported, bool pwrSupState, bool pwrSupPairsCtrl,
    uint8_t psePowerPair, uint8_t powerClass)
{
    const size_t tlvLen = sizeof (OID_802_3) + 1 + 3;
    cFixedByteArray* value = preparePowerViaMDI (tlvLen, portClassPSE, pwrSupSupported, pwrSupState, pwrSupPairsCtrl,
        psePowerPair, powerClass);
    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addDllExtPowerViaMDI (
    bool portClassPSE, bool pwrSupSupported, bool pwrSupState, bool pwrSupPairsCtrl,
    uint8_t psePowerPair, uint8_t powerClass, uint8_t powerType, uint8_t powerSource,
    uint8_t pd4pid, uint8_t powerPrio, double pdRequestedPower, double pseRequestedPower)
{
    const size_t tlvLen = sizeof (OID_802_3) + 1 + 3 + 5;
    cFixedByteArray* value = preparePowerViaMDI (tlvLen, portClassPSE, pwrSupSupported, pwrSupState, pwrSupPairsCtrl,
        psePowerPair, powerClass);

    uint8_t ptsp = ((powerType & 3)   << 6) |
                   ((powerSource & 3) << 4) |
                   ((pd4pid & 1)      << 2) |
                   (powerPrio & 3);
    *value << ptsp;

    *value << ntohs (doubleToPowerValue (pdRequestedPower));
    *value << ntohs (doubleToPowerValue (pseRequestedPower));


    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addMaxFrameSize (uint16_t size)
{
    const size_t tlvLen = sizeof (OID_802_3) + 1 + sizeof (size);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_802_3, sizeof (OID_802_3));
    *value << (uint8_t)TLV_SUBTYPE_802_3::MAX_FRAME_SIZE;
    *value << toBE16 (size);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addEEE (uint16_t txTw, uint16_t rxTw, uint16_t fbTw, uint16_t echoTxTw, uint16_t echoRxTw)
{
    const size_t tlvLen = sizeof (OID_802_3) + 1 + sizeof (txTw) + sizeof (rxTw) + sizeof (fbTw) + sizeof (echoTxTw) + sizeof (echoRxTw);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);

    value->append (OID_802_3, sizeof (OID_802_3));
    *value << (uint8_t)TLV_SUBTYPE_802_3::ENERGY_EFFICIENT_ETH;
    *value << toBE16 (txTw);
    *value << toBE16 (rxTw);
    *value << toBE16 (fbTw);
    *value << toBE16 (echoTxTw);
    *value << toBE16 (echoRxTw);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addEEEFastWake (bool tx, bool rx, bool echoTx, bool echoRx)
{
    const size_t tlvLen = sizeof (OID_802_3) + 1 + 4;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);

    value->append (OID_802_3, sizeof (OID_802_3));
    *value << (uint8_t)TLV_SUBTYPE_802_3::EEE_FAST_WAKE;
    *value << uint8_t (tx ? 1 : 0);
    *value << uint8_t (rx ? 1 : 0);
    *value << uint8_t (echoTx ? 1 : 0);
    *value << uint8_t (echoRx ? 1 : 0);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnDelay (uint32_t portRxDelayLocal, uint32_t portRxDelayRemote,
    uint32_t portTxDelayLocal, uint32_t portTxDelayRemote, uint32_t cableDelay)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + sizeof (portRxDelayLocal) + sizeof (portRxDelayRemote) +
        sizeof (portTxDelayLocal) + sizeof (portTxDelayRemote) + sizeof (cableDelay);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_DELAY;
    *value << toBE32 (portRxDelayLocal);
    *value << toBE32 (portRxDelayRemote);
    *value << toBE32 (portTxDelayLocal);
    *value << toBE32 (portTxDelayRemote);
    *value << toBE32 (cableDelay);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnPortStatus (uint16_t rtc2PortStatus, uint8_t rtc3State,
    bool rtc3Frag, bool rtc3shortPreamp, bool rtc3optimized)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + 2 + 2;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_PORT_STATUS;
    *value << toBE16 (rtc2PortStatus);
    uint16_t rtc3portState =
        uint16_t (rtc3State & 7) |
        uint16_t (rtc3Frag ? 0x1000: 0) |
        uint16_t (rtc3shortPreamp ? 0x2000: 0) |
        uint16_t (rtc3optimized ? 0x8000: 0);
    *value << toBE16 (rtc3portState);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnAlias (const uint8_t* aliasName, uint8_t len)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + len;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_ALIAS;
    value->append (aliasName, len);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnMrpPortStatus (const uint8_t* domainUUID, uint16_t mrrtPortState)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + 16 + sizeof (mrrtPortState);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_MRP_PORT_STATUS;
    value->append (domainUUID, 16);
    *value << toBE16 (mrrtPortState & 3);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnChassisMac (const cMacAddress& chassisMac)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + chassisMac.size();
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_CHASSIS_MAC;
    value->append (chassisMac.get(), chassisMac.size());

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnPtcpStatus (
    const cMacAddress& masterSourceMac, const uint8_t* ptcpSubdomainUUID, const uint8_t* irdataUUID,
    uint32_t lengthOfPeriod, bool lengthOfPeriodValid,
    uint32_t redOrangePeriodBegin, bool redOrangePeriodBeginValid,
    uint32_t orangePeriodBegin, bool orangePeriodBeginValid,
    uint32_t greenPeriodBegin, bool greenPeriodBeginValid)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + 6 + 2*16 + 4*4;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_PTCP_STATUS;
    value->append (masterSourceMac.get(), masterSourceMac.size());
    value->append (ptcpSubdomainUUID, 16);
    value->append (irdataUUID, 16);
    *value << toBE32 (lengthOfPeriod       | uint32_t (lengthOfPeriodValid ? 0x80000000 : 0));
    *value << toBE32 (redOrangePeriodBegin | uint32_t (redOrangePeriodBeginValid ? 0x80000000 : 0));
    *value << toBE32 (orangePeriodBegin    | uint32_t (orangePeriodBeginValid ? 0x80000000 : 0));
    *value << toBE32 (greenPeriodBegin     | uint32_t (greenPeriodBeginValid ? 0x80000000 : 0));

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnMauTypeExtension (uint16_t mauTypeExtension)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + sizeof (mauTypeExtension);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_MAUTYPE_EXT;
    *value << toBE16 (mauTypeExtension);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnMrpInterconnectPortStatus (uint16_t domainID, uint16_t role, uint16_t position)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + sizeof (domainID) + sizeof (role) + sizeof (position);
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_MRPIC_PORT_STATUS;
    *value << toBE16 (domainID) << toBE16 (role) << toBE16 (position);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnNmeDomainUUID (const uint8_t* nmeSubdomainUUID)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + 16;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_NME_DOMAIN_UUID;
    value->append (nmeSubdomainUUID, 16);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnNmeManagementAddr (uint8_t mgtAddrSubtype, const uint8_t* mgtAddr, uint8_t mgtAddrLen)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + mgtAddrLen;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    *value << mgtAddrLen << mgtAddrSubtype;
    value->append (mgtAddr, mgtAddrLen);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnNmeNameUUID (const uint8_t* nmeNameUUID)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + 16;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_NME_NAME_UUID;
    value->append (nmeNameUUID, 16);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addPnNmeParameterUUID (const uint8_t* nmeParameterUUID)
{
    const size_t tlvLen = sizeof (OID_PNO) + 1 + 16;
    cFixedByteArray* value = new cFixedByteArray (tlvLen);
    value->append (OID_PNO, sizeof (OID_PNO));
    *value << (uint8_t)TLV_SUBTYPE_PN::PN_NME_PAR_UUID;
    value->append (nmeParameterUUID, 16);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addRawTLV (uint8_t type, const uint8_t* val, uint16_t valLen /*9 bits are allowed -> 0 - 511 octets*/)
{
    BUG_ON (valLen > 511);
    cFixedByteArray* value = new cFixedByteArray (valLen);
    value->append (val, valLen);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (type, value));
}

void cLldpPacket::addOuiTLV (const uint8_t* oui /*3 bytes*/, uint8_t subtype, const uint8_t* val, uint16_t valLen /* 0 - 507 octets*/)
{
    BUG_ON (valLen > 507);
    cFixedByteArray* value = new cFixedByteArray (valLen+3+1);
    value->append (oui, 3);
    *value << subtype;
    value->append (val, valLen);

    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (OUI, value));
}

void cLldpPacket::addEndTLV ()
{
    cFixedByteArray* value = new cFixedByteArray (1);
    *value << (uint8_t)0;
    m_tlvs.push_back (std::pair<uint8_t, const cFixedByteArray*> (uint8_t(0), value));
}

void cLldpPacket::compile (bool withEndTLV)
{
    // if no destination is set, we use the standard LLDP multicast address
    if (!hasDestMac())
    {
        setDestMac (cMacAddress(1, 0x80, 0xc2, 0, 0, 0x0E));
    }
    setTypeLength (ETHERTYPE_LLDP);

    // copy all TLVs
    for (const auto& tlv : m_tlvs)
    {
        uint16_t type_len;
        type_len = ((uint16_t)(tlv.first & 0x7f) << 9) | ((uint16_t)tlv.second->size() & 0x1ff);
        type_len = toBE16 (type_len);
        appendPayload ((const uint8_t*)&type_len, sizeof (type_len));
        appendPayload (tlv.second->data(), tlv.second->size());
    }

    // End Of LLDPDU TLV
    if (withEndTLV)
    {
        uint16_t end = 0;
        appendPayload ((const uint8_t*)&end, sizeof (end));
    }
}
