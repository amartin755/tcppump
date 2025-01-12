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


#ifndef ASCIIBACKEND_HPP_
#define ASCIIBACKEND_HPP_

#include <cstdio>
#include <string>

#include "filebackend.hpp"


class cAsciiBackend : public cFileBackend
{
public:
    cAsciiBackend (const char* file, bool printPacketNumber, bool printPacketTime, bool hexdump, const char* colSeparator, const char* byteSeparator);
    void write (const cTimeval& sendTime, cEthernetPacket& p);
    void statistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const;
    ~cAsciiBackend ();


private:
    void dump (const void* p, size_t length);
    void write (const char* format, ...);

    FILE*             m_outfile;
    const std::string m_filename;
    const bool        m_printPacketNumber;
    const bool        m_printPacketTime;
    const bool        m_hexdump;
    const std::string m_colSeparator;
    const std::string m_byteSeparator;

    uint64_t    m_writtenPackets;
    uint64_t    m_writtenBytes;
};

#endif /* ASCIIBACKEND_HPP_ */
