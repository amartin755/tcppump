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

#include "arp.hpp"

#include "bug.hpp"
#include "inet.h"
#include "console.hpp"
#include "interface.hpp"

cArp::cArp (cInterface& i) : ifc(i)
{
    // We don't really need an "opened" interface here. This is a sanity check, to accept validated interfaces only.
    BUG_ON (i.isOpen ());
}

bool cArp::resolve (const cIpAddress& ip, cMacAddress& mac)
{
    // TODO implement me
    mac.set("00:de:ad:be:ef:00");
    return false;
}
