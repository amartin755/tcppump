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

#ifndef OUTPUT_HPP_
#define OUTPUT_HPP_

#include <cstdint>

#include "packetdata.hpp"
#include "interface.hpp"
#include "preprocessor.hpp"
#if HAVE_PCAP
#include "pcapfileio.hpp"
#endif


class cOutput
{
public:
    cOutput (const cPreprocessor &preproc);
    void prepare (cInterface &netif, bool realtime, int repeat);
#if HAVE_PCAP
    void prepare (const char* pcapOutFile, int repeat);
#endif
    cPacketData& operator<< (cPacketData& input);
    void statistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const;


private:
#if HAVE_PCAP
    cPcapFileIO outfile;
#endif
    const cPreprocessor &preproc;
    cInterface *netif;
    bool realtimeMode;
    int repeat;

    uint64_t pcapWrittenPackets;
    uint64_t pcapWrittenBytes;
};

#endif /* OUTPUT_HPP_ */
