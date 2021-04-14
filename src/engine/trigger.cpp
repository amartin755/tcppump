/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2021 Andreas Martin (netnag@mailbox.org)
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

#if HAVE_MEMMEM
#include <string.h>
#else
#include <algorithm>
#endif

#include "trigger.hpp"
#include "netinterface.hpp"
#include "timeval.hpp"
#include "dissector.hpp"

cTrigger::cTrigger()
{
    patternFilter = nullptr;
    timeout = 0;
}


cTrigger::~cTrigger()
{
    if (patternFilter)
        delete patternFilter;
    patternFilter = nullptr;
}


bool cTrigger::compileBpfFilter (const char* filter)
{
    return pcapFilter.compile (filter);
}


bool cTrigger::wait (cNetInterface &netif) const
{
    // TODO filtering should be done here and not in interface code
    // --> remove filter parameter in receivePacket function
    // see TODOs in cInstructionParser::compileWait
    cTimeval t;
    t.now();
    int len = 0;
    const uint8_t* p;
    bool match;
    do
    {
        // return on receive AND filter matches OR on error (return==nullptr)
        p = netif.receivePacket (nullptr, &len, &pcapFilter, &t);
        cDissector(p, len).dissect();
        match = p && matchPattern (p, len);
    }
    while (p && !match);

    return match;
}


void cTrigger::setPatternFilter (const uint8_t* pattern, size_t len)
{
    patternFilter = new std::vector<uint8_t>(pattern, pattern + len);
}


void cTrigger::setTimeout (uint32_t t)
{
    timeout = t;
}

bool cTrigger::matchPattern (const uint8_t* packet, int len) const
{
    if (!patternFilter)
        return true;
#if HAVE_MEMMEM
    return !!memmem (packet, len, patternFilter->data(), patternFilter->size());
#else
    auto it = std::search (packet, packet + len, patternFilter->begin(), patternFilter->end());
    return it != (packet + len);
#endif
}
