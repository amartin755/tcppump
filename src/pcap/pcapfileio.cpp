// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2021 Andreas Martin (netnag@mailbox.org)
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


#include <cstdio>

#include "libpcap.h"

#include "bug.hpp"
#include "pcapfileio.hpp"
#include "console.hpp"


cPcapFileIO::cPcapFileIO ()
{
    m_modeWrite  = false;
    m_fileHandle = NULL;
    m_dumper     = NULL;
    m_fileError  = false;
    m_path       = NULL;
    m_eof        = false;
    m_firstRead  = false;
}


cPcapFileIO::~cPcapFileIO ()
{
    close ();
}


bool cPcapFileIO::open (const char* path, bool write)
{
    BUG_ON (m_fileHandle);

    char errbuf[PCAP_ERRBUF_SIZE] = {0};

    if (!write)
    {
        m_fileHandle = pcap_open_offline (path, errbuf);
        m_firstRead  = true;
    }
    else
    {
        m_fileHandle = pcap_open_dead (DLT_EN10MB, 65535);
        if (m_fileHandle)
        {
            m_dumper = pcap_dump_open (m_fileHandle, path);
            if (!m_dumper)
            {
                printError (pcap_geterr (m_fileHandle));
                pcap_close (m_fileHandle);
                m_fileHandle = NULL;

                return false;
            }
        }
    }

    if (!m_fileHandle)
    {
        printError (errbuf);

        return false;
    }

    m_modeWrite = write;
    m_path      = path;
    m_eof       = false;
    m_offset.clear ();

    return true;
}


void cPcapFileIO::close ()
{
    if (m_dumper)
        pcap_dump_close (m_dumper);
    if (m_fileHandle)
        pcap_close (m_fileHandle);

    m_fileHandle = NULL;
    m_dumper     = NULL;
    m_path       = NULL;
    m_fileError  = false;
    m_eof        = false;
}


bool cPcapFileIO::read (struct pcap_pkthdr **header, const u_char **data)
{
    BUG_ON (!m_fileHandle);
    BUG_ON (m_modeWrite);

    if (!m_fileError && !m_eof)
    {
        int res = pcap_next_ex(m_fileHandle, header, data);
        m_fileError = res == 0 || res == -1;
        m_eof = res == -2;

        if (m_fileError)
        {
            Console::PrintError ("Could not read file %s.\n", m_path);
            printError (pcap_geterr (m_fileHandle));
        }
        if (m_firstRead)
        {
            m_offset = (*header)->ts;
            m_firstRead = false;
        }
        (*header)->ts = cTimeval ((*header)->ts).sub(m_offset).timeval();
    }
    return !(m_fileError || m_eof);
}


uint8_t* cPcapFileIO::read (cTimeval* timestamp, int* len)
{
    struct pcap_pkthdr *header;
    const u_char *pkt_data;

    if (read (&header, &pkt_data))
    {
        timestamp->set (header->ts);
        *len = (int)header->len;

        return (uint8_t*)pkt_data;
    }

    *len = 0;
    return NULL;
}


bool cPcapFileIO::write (const cTimeval& timestamp, const uint8_t* frame, int len, bool absoluteTimestamp)
{
    BUG_ON (!m_fileHandle);
    BUG_ON (!m_dumper);
    BUG_ON (!m_modeWrite);

    if (!m_fileError)
    {
        struct pcap_pkthdr hdr;

        if (absoluteTimestamp)
            m_offset.set (timestamp);
        else
            m_offset.add (timestamp);
        hdr.ts  = m_offset.timeval();
        hdr.len = hdr.caplen = len;

        pcap_dump ((u_char*)m_dumper, &hdr, (u_char*)frame);

        // as pcap_dump doesn't return an error code, we have to do error checking on dumpers underlying FILE pointer
        m_fileError = ferror (pcap_dump_file (m_dumper));
        if (m_fileError)
        {
            Console::PrintError ("Could not write file %s.\n", m_path);
        }
    }
    return !m_fileError;
}

// FIXME same code as in cPcap!
void cPcapFileIO::printError (const char* err)
{
    Console::PrintError ("pcap error: %s\n", err);
}

#ifdef WITH_UNITTESTS
#include <cstring>
#include <cstdlib>
#include "parsehelper.hpp"
void cPcapFileIO::unitTest (const char* file)
{
    Console::PrintDebug("-- " __FILE__ " --\n");

    cPcapFileIO obj;

    BUG_IF_NOT (!obj.m_modeWrite);
    BUG_IF_NOT (!obj.m_fileHandle);
    BUG_IF_NOT (!obj.m_dumper);
    BUG_IF_NOT (!obj.m_path);
    BUG_IF_NOT (!obj.m_fileError);
    BUG_IF_NOT (!obj.m_eof);

    BUG_IF_NOT (!obj.open("nofile"));
    BUG_IF_NOT (!obj.m_modeWrite);
    BUG_IF_NOT (!obj.m_fileHandle);
    BUG_IF_NOT (!obj.m_dumper);
    BUG_IF_NOT (!obj.m_path);
    BUG_IF_NOT (!obj.m_fileError);
    BUG_IF_NOT (!obj.m_eof);

    BUG_IF_NOT (!obj.open("nofile", false));
    BUG_IF_NOT (!obj.m_modeWrite);
    BUG_IF_NOT (!obj.m_fileHandle);
    BUG_IF_NOT (!obj.m_dumper);
    BUG_IF_NOT (!obj.m_path);
    BUG_IF_NOT (!obj.m_fileError);
    BUG_IF_NOT (!obj.m_eof);

    struct frame
    {
        uint64_t t;
        uint8_t* bin;
        size_t binlen;
        const char* txt;
    };

    frame indata[] = {
        {0, NULL, 0, "5c353beac27f58946bd31d5c0800450000412db8000080118b1ac0a80088c0a80001ca040035002d238b7f9901000001000000000000037777770b736f75726365666f726765036e65740000010001"},
        {25636, NULL, 0, "58946bd31d5c5c353beac27f0800450000515d3e000040119b84c0a80001c0a800880035ca04003d019b7f9981800001000100000000037777770b736f75726365666f726765036e65740000010001c00c000100010000012c0004d822b53f"},
        {26080, NULL, 0, "5c353beac27f58946bd31d5c0800450000412db9000080118b19c0a80088c0a80001dbe90035002d735a02e501000001000000000000037777770b736f75726365666f726765036e657400001c0001"},
        {51897, NULL, 0, "58946bd31d5c5c353beac27f0800450000935d3f000040119b41c0a80001c0a800880035dbe9007f2f2702e581800001000000010000037777770b736f75726365666f726765036e657400001c0001c010000600010000003c0046036e7331037030330664796e656374c01c0a686f73746d617374657204636f72700b736f75726365666f72676503636f6d00780d090d00000e100000025800093a800000003c"},
        {4394680, NULL, 0, "01005e7ffffa5c353beac27f0800450001fe44ca00000211c181c0a80001effffffabd6a076c01ea4f6a4e4f54494659202a20485454502f312e310d0a484f53543a203233392e3235352e3235352e3235303a313930300d0a43414348452d434f4e54524f4c3a206d61782d6167653d36300d0a6c4f434154494f4e3a20687474703a2f2f3139322e3136382e302e313a353030302f726f6f74446573632e786d6c0d0a5345525645523a20436f6d70616c2042726f616462616e64204e6574776f726b732c20496e632f4c696e75782f322e362e33392e332055506e502f312e31204d696e6955506e50642f312e370d0a4e543a2075726e3a736368656d61732d75706e702d6f72673a6465766963653a496e7465726e6574476174657761794465766963653a310d0a55534e3a20757569643a41333733353143352d383532312d346332342d413433452d3543333533424541433237463a3a75726e3a736368656d61732d75706e702d6f72673a6465766963653a496e7465726e6574476174657761794465766963653a310d0a4e54533a20737364703a616c6976650d0a4f50543a2022687474703a2f2f736368656d61732e75706e702e6f72672f75706e702f312f302f223b206e733d30310d0a30312d4e4c533a20310d0a424f4f5449442e55504e502e4f52473a20310d0a434f4e46494749442e55504e502e4f52473a20313333370d0a0d0a"}
    };

    for (unsigned n = 0; n < sizeof (indata) / sizeof (indata[0]); n++)
    {
        indata[n].bin = cParseHelper::hexStringToBin (indata[n].txt, 0, indata[n].binlen);
        BUG_IF_NOT (indata[n].bin);
    }

    uint8_t* f;
    int len;
    cTimeval t;
    int n = 0;

    BUG_IF_NOT (obj.open(file, false));
    BUG_IF_NOT (!obj.m_modeWrite);
    BUG_IF_NOT (obj.m_fileHandle);
    BUG_IF_NOT (!obj.m_dumper);
    BUG_IF_NOT (obj.m_path);
    BUG_IF_NOT (!obj.m_fileError);
    BUG_IF_NOT (!obj.m_eof);
    BUG_IF_NOT (obj.m_offset.isNull());

    while ((f = obj.read (&t, &len)) != NULL)
    {
        BUG_IF_NOT (t.us() == indata[n].t);
        BUG_IF_NOT ((size_t)len == indata[n].binlen);
        BUG_IF_NOT (!memcmp (f, indata[n].bin, indata[n].binlen));
        n++;
    }

    BUG_IF_NOT (!obj.m_fileError);
    BUG_IF_NOT (obj.m_eof);
    BUG_IF_NOT (n == (sizeof (indata) / sizeof (indata[0])));

    obj.close ();
    BUG_IF_NOT (!obj.m_modeWrite);
    BUG_IF_NOT (!obj.m_fileHandle);
    BUG_IF_NOT (!obj.m_dumper);
    BUG_IF_NOT (!obj.m_path);
    BUG_IF_NOT (!obj.m_fileError);
    BUG_IF_NOT (!obj.m_eof);

    for (unsigned i = 0; i < sizeof (indata) / sizeof (indata[0]); i++)
    {
        delete[] (indata[i].bin);
    }

    //TODO unit tests
    // - open known pcap file
    // - read contents and write to new pcap file
    // - both files have to be identical
}
#endif /* WITH_UNITTESTS */

