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


#ifndef ARP_HPP_
#define ARP_HPP_

#include "macaddress.hpp"
#include "ipaddress.hpp"

class cNetInterface;

class cArp
{
public:
    cArp (cNetInterface& ifc);
    bool resolve (const cIPv4& ip, cMacAddress& mac);

private:
    cNetInterface& ifc;
};


#endif /* ARP_HPP_ */
