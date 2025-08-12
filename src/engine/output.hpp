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


#ifndef OUTPUT_HPP_
#define OUTPUT_HPP_

#include <cstdint>

#include "packetdata.hpp"
#include "netinterface.hpp"
#include "preprocessor.hpp"


class cFileBackend;

class cOutput
{
public:
    cOutput (const cPreprocessor &preproc);
    ~cOutput ();
    void prepare (cNetInterface &netif, bool realtime, int repeat);
    void prepare (const char* outfile, const char* format, int repeat);
    cPacketData& operator<< (cPacketData& input);
    void statistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const;


private:
    cFileBackend* m_outfile;
    inline void processPacket (const cTimeval& sendTime, cEthernetPacket& p);
    const cPreprocessor &m_preproc;
    cNetInterface *m_netif;
    bool m_realtimeMode;
    int m_repeat;
};

#endif /* OUTPUT_HPP_ */
