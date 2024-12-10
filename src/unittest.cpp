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

#include "bug.hpp"
#include "console.hpp"
#include "timeval.hpp"
#include "cmdline.hpp"
#include "pcapfileio.hpp"
#include "pcapfilter.hpp"
#include "ethernetpacket.hpp"
#include "arppacket.hpp"
#include "common/bug.hpp"
#include "ipv4packet.hpp"
#include "parameterlist.hpp"
#include "instructionparser.hpp"
#include "sleep.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "parsehelper.hpp"
#include "inetchecksum.hpp"
#include "random.hpp"
#if HAVE_MSVC
#include <crtdbg.h>
#endif


int main (void)
{
#if HAVE_MSVC
        if(!IsDebuggerPresent())
        {
            _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
            _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
        }
#endif

    Console::SetPrintLevel(Console::Debug);
    cRandom::create();
    try
    {
        tcppump::SleepInit ();
        tcppump::SleepUnitTest ();
        cIPv4::unitTest ();
        cIPv6::unitTest ();
        cMacAddress::unitTest ();
        cTimeval::unitTest ();
        cCmdline::unitTest ();
        cInetChecksum::unitTest();
        cEthernetPacket::unitTest ();
        cArpPacket::unitTest ();
        cIPv4Packet::unitTest ();
        cParseHelper::unitTest ();
        cParameterList::unitTest ();
        cInstructionParser::unitTest ();

#if HAVE_PCAP
        cPcapFileIO::unitTest ("../src/test/readtest.pcap");
        cPcapFilter::unitTest ();
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
