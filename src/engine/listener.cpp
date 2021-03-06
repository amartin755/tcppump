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


#if HAVE_MEMMEM
#include <string.h>
#else
#include <algorithm>
#endif

#include "listener.hpp"
#include "netinterface.hpp"
#include "timeval.hpp"
#include "dissector.hpp"

cListener::cListener()
{
    patternFilter = nullptr;
    timeout = 0;
}


cListener::~cListener()
{
    if (patternFilter)
        delete patternFilter;
    patternFilter = nullptr;
}


bool cListener::compileBpfFilter (const char* filter)
{
    return pcapFilter.compile (filter);
}


bool cListener::wait (cNetInterface &netif) const
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


void cListener::setPatternFilter (const uint8_t* pattern, size_t len)
{
    patternFilter = new std::vector<uint8_t>(pattern, pattern + len);
}


void cListener::setTimeout (uint32_t t)
{
    timeout = t;
}

bool cListener::matchPattern (const uint8_t* packet, int len) const
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
