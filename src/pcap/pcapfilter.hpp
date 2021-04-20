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


#ifndef PCAPFILTER_HPP_
#define PCAPFILTER_HPP_


#include <cstdint>
#include <list>
#include <pcap.h>


class cPcapFilter
{
public:
    cPcapFilter ();
    cPcapFilter (pcap_t* ifc);
    virtual ~cPcapFilter ();
    bool compile (const char* filter);
    bool compile (bool tcp, bool udp,
                           const std::list<const char*>* ethertypes,
                           const std::list<const char*>* hostsMAC,
                           const std::list<const char*>* hostsIP);
    bool apply (void);
    bool remove (void);
    bool match (const struct pcap_pkthdr *h, const u_char *pkt) const;

#ifdef WITH_UNITTESTS
    static void unitTest ();
#endif

private:
    void resetBPF (void);

    pcap_t* ifcHandle;
    bpf_program bpfCode;
    bool freeIfcHandle;
};

#endif /* PCAPFILTER_HPP_ */
