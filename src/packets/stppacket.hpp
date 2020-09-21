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

#ifndef STP_PACKET_H_
#define STP_PACKET_H_

#include <cstring>
#include "inet.h"
#include "ethernetpacket.hpp"

/* https://en.wikipedia.org/wiki/Spanning_Tree_Protocol

 1. Protocol ID:       2 bytes (0x0000 IEEE 802.1D)
 2. Version ID:        1 byte (0x00 Config & TCN / 0x02 RST / 0x03 MST / 0x04 SPT  BPDU)
 3. BPDU Type:         1 byte (0x00 STP Config BPDU, 0x80 TCN BPDU, 0x02 RST/MST Config BPDU)
 4. Flags:             1 byte
   bits  : usage
       1 : 0 or 1 for Topology Change
       2 : 0 (unused) or 1 for Proposal in RST/MST/SPT BPDU
     3-4 : 00 (unused) or
           01 for Port Role Alternate/Backup in RST/MST/SPT BPDU
           10 for Port Role Root in RST/MST/SPT BPDU
           11 for Port Role Designated in RST/MST/SPT BPDU
       5 : 0 (unused) or 1 for Learning in RST/MST/SPT BPDU
       6 : 0 (unused) or 1 for Forwarding in RST/MST/SPT BPDU
       7 : 0 (unused) or 1 for Agreement in RST/MST/SPT BPDU
       8 : 0 or 1 for Topology Change Acknowledgement
 5. Root ID:           8 bytes (CIST Root ID in MST/SPT BPDU)
   bits  : usage
     1-4 : Root Bridge Priority
    5-16 : Root Bridge System ID Extension
   17-64 : Root Bridge MAC Address
 6. Root Path Cost:    4 bytes (CIST External Path Cost in MST/SPT BPDU)
 7. Bridge ID:         8 bytes (CIST Regional Root ID in MST/SPT BPDU)
   bits  : usage
     1-4 : Bridge Priority
    5-16 : Bridge System ID Extension
   17-64 : Bridge MAC Address
  8. Port ID:          2 bytes (4 bits prio, 12 bits port number) port number = 0 is not allowed
  9. Message Age:      2 bytes in 1/256 secs
 10. Max Age:          2 bytes in 1/256 secs
 11. Hello Time:       2 bytes in 1/256 secs
 12. Forward Delay:    2 bytes in 1/256 secs
 13. Version 1 Length: 1 byte (0x00 no ver 1 protocol info present. RST, MST, SPT BPDU only)
 14. Version 3 Length: 2 bytes (MST, SPT BPDU only)

 The TCN BPDU includes fields 1-3 only.
*/


#pragma pack(1)
typedef struct
{
    uint16_t  prio_ext;
    cMacAddress::mac_t systemId;

    void set (unsigned prio, unsigned ext, const cMacAddress& mac)
    {
    	prio_ext = htons (((prio & 0x0f) << 12) | (ext & 0x0fff));
    	mac.get (&systemId);
    }
}bridge_id_t;

typedef struct
{
    uint16_t    protocol;
    uint8_t     version;
    uint8_t     type;
    uint8_t     flags;
    bridge_id_t root;
    uint32_t    rootPathCost;
    bridge_id_t bridge;
    uint16_t    portId;
    uint16_t    messageAge;
    uint16_t    maxAge;
    uint16_t    helloTime;
    uint16_t    forwardDelay;

    void setPortId (unsigned prio, uint16_t number)
    {
    	portId = htons (((prio & 0x0f) << 12) | (number & 0x0fff));
    }
    void setFlags (bool topoChange, bool topoChangeAck)
    {
    	flags &= 0x81;
    	flags |= (((int)topoChangeAck) << 7) | (int)topoChange;
    }

}bpdu_t;

typedef struct
{
    uint16_t    protocol;
    uint8_t     version;
    uint8_t     type;
}tcnpdu_t;
#pragma pack()

class cStpPacket : public cEthernetPacket
{
public:
	cStpPacket();
    void compile (const cMacAddress& srcMac, unsigned rootBridgePrio, unsigned rootBridgeId, const cMacAddress& rootBridgeMac, uint32_t pathCost,
    		unsigned bridgePrio, unsigned bridgeId, const cMacAddress& bridgeMac, unsigned portPrio, unsigned portNumber,
    		double msgAge, double maxAge, double helloTime, double forwardDelay, bool topoChange, bool topoChangeAck);



private:
    uint16_t toTime (double seconds) const;
};

#endif /* VRRP_PACKET_H_ */
