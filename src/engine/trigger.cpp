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


#include "trigger.hpp"
#include "netinterface.hpp"
#include "timeval.hpp"


bool cTrigger::compileFilter (const char* filter)
{
    return pcapFilter.compile (filter);
}


bool cTrigger::wait (cNetInterface &netif)
{
    // TODO filtering should be done here and not in interface code
    // --> remove filter parameter in receivePacket function
    // see TODOs in cInstructionParser::compileWait
    cTimeval t;
    t.now();
    return !!netif.receivePacket(nullptr, nullptr, &pcapFilter, &t);
}
