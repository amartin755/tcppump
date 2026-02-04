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
#include <chrono>
#include <limits>

#include "bug.hpp"
#include "console.hpp"
#include "instructionparser.hpp"
#include "sleep.hpp"
#include "settings.hpp"


int main (void)
{
    Console::SetPrintLevel(Console::Debug);
    cRandom::create();
    tcppump::SleepInit ();
    try
    {
        BUG_ON (!cSettings::get().setMyIPv4 ("1.2.3.4"));
        BUG_ON (!cSettings::get().setMyIPv6 ("2001:0db8:85a3:0000:0000:8a2e:0370:7334"));
        BUG_ON (!cSettings::get().setMyMAC  ("ca:fe:ba:be:00:01"));

        cInstructionParser::cResult result;
        cInstructionParser parser (false);

        const char* packets[] = {
            "udp(dmac=11:22:33:44:55:66, dip=1.2.3.4, sport=1, dport=2, payload=11223344556677)",
            "udp(vid=42, dmac=11:22:33:44:55:66, dip=1.2.3.4, sport=1, dport=2, payload=11223344556677)",
            "udp(dmac=12:23:34:34:44:44, dip=1.2.3.4, sport=1234, dport=2345, payload=<raw(byte=0x55, be16=0x1234, le16=0x1234, be32=0x11223344)>)",
            "lldp()"
        };

        for (const char* packet : packets)
        {
            int64_t min = std::numeric_limits<int64_t>::max();
            fprintf (stderr, "%s\n", packet);
            for (int i = 0; i < 1000000; i++)
            {
                auto t1 = std::chrono::high_resolution_clock::now();

                parser.parse (packet, result);

                auto t2 = std::chrono::high_resolution_clock::now();
                auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
                if (elapsed_ns.count() < min)
                {
                    min = elapsed_ns.count();
                }
            }
            fprintf (stderr, "-> %lld ns\n", (long long)min);
        }
    }
    catch (...)
    {
        BUG ("unhandled exception");
    }
    // every failure will lead to abort, thus if we see this output, all tests have passed
    fprintf (stderr, "\n --- performance tests finished successfully !!! --- \n");

    cRandom::destroy();
    return 0;
}
