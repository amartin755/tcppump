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


#include "pcapbackend.hpp"
#include "fileioexception.hpp"
#include "ethernetpacket.hpp"


cPcapBackend::cPcapBackend (const char* file) : m_pcapWrittenPackets(0), m_pcapWrittenBytes(0)
{
    if (!m_outfile.open (file, true))
        throw FileIOException (FileIOException::OPEN, file);

}

void cPcapBackend::write (const cTimeval& sendTime, cEthernetPacket& p)
{
    if (!m_outfile.write (sendTime, p.get(), (int)p.getLength(), true))
        throw FileIOException (FileIOException::WRITE, m_outfile.name());
    m_pcapWrittenPackets++;
    m_pcapWrittenBytes += (uint64_t)p.getLength ();
}

void cPcapBackend::statistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const
{
    sentPackets = m_pcapWrittenPackets;
    sentBytes   = m_pcapWrittenBytes;
    duration    = 0;
}

cPcapBackend::~cPcapBackend ()
{

}
