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


#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <vector>
#include "linkable.hpp"
#include "pcapfilter.hpp"

class cNetInterface;

class cListener : public cLinkable
{
public:
    cListener();
    ~cListener();

    bool wait (cNetInterface &netif) const;

    bool compileBpfFilter (const char* filter);
    void setPatternFilter (const uint8_t* pattern, size_t len);
    void setTimeout (uint32_t timeout);

private:
    bool matchPattern (const uint8_t* packet, int len) const;
    cPcapFilter pcapFilter;
    std::vector<uint8_t>* patternFilter;
    uint32_t timeout;
};

#endif /* LISTENER_HPP */
