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


#include <winsock2.h>
#include <iphlpapi.h>

#include "arp.hpp"

#include "bug.hpp"
#include "inet.h"
#include "console.hpp"
#include "netinterface.hpp"

cArp::cArp (cNetInterface& i) : ifc(i)
{
}

bool cArp::resolve (const cIpAddress& ip, cMacAddress& mac)
{
    ULONG macAddr[2];    // SendARP expects at least two ULONGs
    ULONG macLen = 6;
    cIpAddress ownIp;

    // We don't really need an "opened" interface here. This is a sanity check, to accept validated interfaces only.
    BUG_ON (!ifc.isOpen ());
    ifc.getIPv4(ownIp);

    DWORD ret = ::SendARP(ip.get().s_addr, ownIp.get().s_addr, (PULONG)macAddr, (PULONG)&macLen);

    if (ret == NO_ERROR && (size_t)macLen == mac.size())
    {
        mac.set (macAddr, macLen);
        return true;
    }
    else
    {
        Console::PrintDebug ("SendARP returned %u\n", (unsigned)ret);
    }

    return false;
}
