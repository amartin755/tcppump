// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
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

#include "bug.hpp"
#include "console.hpp"
#include "timeval.hpp"
#include "cmdline.hpp"
#include "pcapfileio.hpp"
#include "ethernetpacket.hpp"
#include "arppacket.hpp"
#include "bug.hpp"
#include "ippacket.hpp"
#include "parameterlist.hpp"
#include "instructionparser.hpp"
#include "sleep.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "parsehelper.hpp"
#include "inetchecksum.hpp"
#include "random.hpp"
#include "bytearray.hpp"
#include "uuid.hpp"
#include "md5.hpp"
#if HAVE_MSVC
#include <crtdbg.h>
#endif


int main (int argc, char* argv[])
{
#if HAVE_MSVC
        if(!IsDebuggerPresent())
        {
            _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
            _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
        }
#endif
    if (argc != 2)
    {
        fprintf (stderr, "usage: %s <testfile-path>\n", argv[0]);
        return 1;
    }
    Console::SetPrintLevel(Console::Debug);
    cRandom::create();
    try
    {
        tcppump::SleepInit ();
        tcppump::SleepUnitTest ();
        cByteArray::unitTest ();
        cUUID::unitTest ();
        cMD5::unitTest ();
        cRandom::unitTest ();
        cIPv4::unitTest ();
        cIPv6::unitTest ();
        cMacAddress::unitTest ();
        cTimeval::unitTest ();
        cInetChecksum::unitTest();
        cEthernetPacket::unitTest ();
        cArpPacket::unitTest ();
        cIPPacket::unitTest ();
        cParseHelper::unitTest ();
        cParameterList::unitTest ();
        cInstructionParser::unitTest ();

#if HAVE_PCAP
        cPcapFileIO::unitTest (argv[1]);
#endif
    }
    catch (...)
    {
        BUG ("unhandled exception");
    }
    // every failure will lead to abort, thus if we see this output, all tests have passed
    fprintf (stderr, "\n --- unit tests finished successfully !!! --- \n");

    cRandom::destroy();
    return 0;
}
