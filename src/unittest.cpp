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


#include <cstdio>

#include "bugon.h"
#include "console.hpp"
#include "timeval.hpp"
#include "cmdline.hpp"
#include "pcapfileio.hpp"
#include "ethernetpacket.hpp"
#include "arppacket.hpp"
#include "ipv4packet.hpp"
#include "parameterlist.hpp"
#include "instructionparser.hpp"
#include "sleep.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "parsehelper.hpp"
#include "inetchecksum.hpp"
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
    try
    {
        tcppump::SleepInit ();
        tcppump::SleepUnitTest ();
        cIpAddress::unitTest ();
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
#endif
    }
    catch (...)
    {
        BUG_ON ("unhandled exception" == 0);
    }
    // every failure will lead to assert, thus if we see this output, all tests have passed
    printf ("\n --- unit tests finished successfully !!! --- \n");

    return 0;
}