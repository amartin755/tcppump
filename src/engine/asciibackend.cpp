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


#include <cstdarg>
#include <cstdio>

#include "asciibackend.hpp"
#include "fileioexception.hpp"
#include "ethernetpacket.hpp"
#include "dissector.hpp"

cAsciiBackend::cAsciiBackend (const char* file, bool printPacketNumber,
    bool printPacketTime, bool hexdump, const char* colSeparator, const char* byteSeparator)
    : m_outfile (stdout),
      m_filename (file),
      m_printPacketNumber (printPacketNumber),
      m_printPacketTime (printPacketTime),
      m_hexdump (hexdump),
      m_colSeparator (colSeparator),
      m_byteSeparator (byteSeparator),
      m_writtenPackets (0),
      m_writtenBytes (0)
{
    if (std::strcmp (file, "-"))
        if ((m_outfile = fopen (file, "wt")) == NULL)
            throw FileIOException (FileIOException::OPEN, file);

}

cAsciiBackend::~cAsciiBackend ()
{
    if (m_outfile && m_outfile != stdout)
    {
        fclose (m_outfile);
        m_outfile = nullptr;
    }
}

void cAsciiBackend::write (const cTimeval& sendTime, cEthernetPacket& p)
{
    struct timeval t = sendTime.timeval();
    const uint8_t* data = p.get();

    if (m_printPacketNumber)
    {
        write ("%5lu%s", (unsigned long)(m_writtenPackets + 1), m_colSeparator.c_str());
    }

    if (m_printPacketTime)
    {
        write ("%jd.%06lu%s", (intmax_t)t.tv_sec, t.tv_usec, m_colSeparator.c_str());
    }

    if (m_hexdump)
    {
        write ("\n");
        dump (data, p.getLength());
        m_writtenPackets++;
        return;
    }

    for (size_t n = 0; n < p.getLength(); n++)
    {
        write ("%02x%s", unsigned(*data++), m_byteSeparator.c_str());
    }
    write ("\n");
    m_writtenPackets++;
}

void cAsciiBackend::statistic (uint64_t& sentPackets, uint64_t& sentBytes, double& duration) const
{
    sentPackets = m_writtenPackets;
    sentBytes   = m_writtenBytes;
    duration    = 0;
}

// based on https://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
void cAsciiBackend::dump (const void* p, size_t length)
{
    unsigned i;
    unsigned char buff[17];
    const uint8_t *pc = (const uint8_t*)p;


    if (length == 0)
    {
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < length; i++)
    {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0)
        {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                write ("  %s\n", buff);

            // Output the offset.
            write ("%04x ", i);
        }

        // Now the hex code for the specific character.
        write (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0)
    {
        write ("   ");
        i++;
    }

    // And print the final ASCII bit.
    write ("  %s\n", buff);
}

void cAsciiBackend::write (const char* format, ...)
{
    va_list args;

    va_start (args, format);
    int ret = vfprintf (m_outfile, format, args);
    va_end (args);

    if (ret <= 0)
        throw FileIOException (FileIOException::WRITE, m_filename.c_str());
    m_writtenBytes += ret;
}