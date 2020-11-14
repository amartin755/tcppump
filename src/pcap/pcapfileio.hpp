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


#ifndef PCAPFILEIO_HPP_
#define PCAPFILEIO_HPP_

#if HAVE_PCAP

#include <cstdint>
#include <pcap.h>

#include "timeval.hpp"

class cPcapFileIO
{
public:
    cPcapFileIO ();
    ~cPcapFileIO ();
#ifdef WITH_UNITTESTS
    static void unitTest (const char* file);
#endif

    bool open (const char* path, bool write = false);
    void close ();
    bool read (struct pcap_pkthdr **, const u_char **);
    uint8_t* read (cTimeval* timestamp, int* len);
    bool write (const cTimeval& timestamp, const uint8_t* frame, int len, bool absoluteTimestamp = true);
    bool error () const {return fileError;};
    const char* name (void) const {return path;};

private:
    void printError (const char* err);

    bool modeWrite;
    pcap_t *fileHandle;
    pcap_dumper_t* dumper;
    const char* path;
    bool fileError;
    bool eof;
    cTimeval offset;
};

#endif /* HAVE_PCAP */
#endif /* PCAPFILEIO_HPP_ */
