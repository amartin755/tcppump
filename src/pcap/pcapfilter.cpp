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



#include <string>
#include <cstring>

#include "pcapfilter.hpp"
#include "bug.hpp"
#include "console.hpp"



cPcapFilter::cPcapFilter ()
{
    ifcHandle = pcap_open_dead (DLT_EN10MB, 65536);
    BUG_ON (!ifcHandle); // must never fail

    freeIfcHandle = true; // remember to close the handle in destructor

    resetBPF ();
}

cPcapFilter::cPcapFilter (pcap_t* ifc) : ifcHandle(ifc)
{
    freeIfcHandle = false;
    resetBPF ();
}

cPcapFilter::~cPcapFilter ()
{
    if (bpfCode.bf_len)
        pcap_freecode (&bpfCode);

    BUG_ON (!ifcHandle);
    if (freeIfcHandle)
        pcap_close (ifcHandle);
}

bool cPcapFilter::compile (bool tcp, bool udp,
                                   const std::list<const char*>* ethertypes,
                                   const std::list<const char*>* hostsMAC,
                                   const std::list<const char*>* hostsIP)
{
    std::string ethHosts;
    std::string ipHosts;
    std::string ethTypes;

    if (ethertypes)
    {
        bool first = true;
        ethTypes = "(ether proto ";
        for (const auto &ethertype : *ethertypes)
        {
            if (!first)
                ethTypes += " or ";
            ethTypes += ethertype;
            first = false;
        }
        ethTypes += ")";
    }
    if (hostsMAC)
    {
        bool first = true;
        ethHosts = "(ether src ";
        for (const auto &mac : *hostsMAC)
        {
            if (!first)
                ethHosts += " or ";
            ethHosts += mac;
            first = false;
        }
        ethHosts += ")";
    }
    if (hostsIP)
    {
        bool first = true;
        ipHosts = "(src host ";
        for (const auto &ip : *hostsIP)
        {
            if (!first)
                ipHosts += " or ";
            ipHosts += ip;
            first = false;
        }
        ipHosts += ")";
    }

    std::string bpf;
    bool hasEthHosts = !!ethHosts.length();
    bool hasIpHosts  = !!ipHosts.length();
    bool hasEthTypes = !!ethTypes.length();

    if (hasEthHosts || hasIpHosts)
    {
        bpf = "(";
        if (hasEthHosts)
        {
            bpf += ethHosts;
        }
        if (hasIpHosts)
        {
            bpf += hasEthTypes ?  " or " + ipHosts: ipHosts;
        }
        bpf += ")";
    }
    if (hasEthTypes || tcp || udp)
    {
        if (hasEthHosts || hasIpHosts)
            bpf += " and ";
        bpf += "(";

        if (hasEthTypes)
            bpf += ethTypes;
        if (tcp)
            bpf += hasEthTypes ? " or tcp" : " tcp";
        if (udp)
            bpf += hasEthTypes || tcp ? " or udp" : " udp";

        bpf += ")";
    }

    // (macs or ips) and (ethertypes or tcp or udp)
    Console::PrintDebug ("pcap filter: %s\n", bpf.c_str());

    return compile (bpf.c_str());
}


bool cPcapFilter::compile (const char* filter)
{
    if (bpfCode.bf_len)
    {
        pcap_freecode (&bpfCode);
        resetBPF ();
    }
    if (pcap_compile (ifcHandle, &bpfCode, filter, 1, 0) < 0)
    {
        Console::PrintError ("Unable to compile the pcap packet filter: %s\n", pcap_geterr (ifcHandle));
        return false;
    }
    return true;
}


bool cPcapFilter::apply (void)
{
    bool ret = true;
    if (pcap_setfilter (ifcHandle, &bpfCode) < 0)
    {
        Console::PrintError ("Error setting the pcap filter: %s\n", pcap_geterr (ifcHandle));
        ret =  false;
    }
    pcap_freecode (&bpfCode);
    resetBPF ();

    return ret;
}


bool cPcapFilter::remove (void)
{
    return compile ("") && apply();
}


void cPcapFilter::resetBPF (void)
{
    std::memset(&bpfCode, 0, sizeof(bpfCode));
}


bool cPcapFilter::match (const struct pcap_pkthdr *h, const u_char *pkt) const
{
    // if no filter is set, ALL packets match
    return (!bpfCode.bf_len) ? true : !!pcap_offline_filter ((struct bpf_program*)&bpfCode, h, pkt);
}


#ifdef WITH_UNITTESTS
void cPcapFilter::unitTest ()
{
    std::list<const char*> ethers;
    std::list<const char*> macs;
    std::list<const char*> ips;
    ethers.push_back ("0x1234");
    macs.push_back ("12:34:56:78:90:AB");
    ips.push_back ("1.2.3.4");

    cPcapFilter obj;

    assert (!obj.compile ("kkdkdkdk"));
    assert (obj.compile ("tcp or udp"));

    assert (obj.compile (false, false, nullptr, nullptr, nullptr));
    assert (obj.compile (true, false, nullptr, nullptr, nullptr));
    assert (obj.compile (false, true, nullptr, nullptr, nullptr));
    assert (obj.compile (true, true, nullptr, nullptr, nullptr));

    assert (obj.compile (false, false, &ethers, nullptr, nullptr));
    assert (obj.compile (false, false, nullptr, &macs, nullptr));
    assert (obj.compile (false, false, nullptr, nullptr, &ips));
    assert (obj.compile (true, false, &ethers, nullptr, nullptr));
    assert (obj.compile (true, false, nullptr, &macs, nullptr));
    assert (obj.compile (true, false, nullptr, nullptr, &ips));
    assert (obj.compile (false, true, &ethers, nullptr, nullptr));
    assert (obj.compile (false, true, nullptr, &macs, nullptr));
    assert (obj.compile (false, true, nullptr, nullptr, &ips));
    assert (obj.compile (true, true, &ethers, nullptr, nullptr));
    assert (obj.compile (true, true, nullptr, &macs, nullptr));
    assert (obj.compile (true, true, nullptr, nullptr, &ips));
    assert (obj.compile (true, true, &ethers, &macs, nullptr));
    assert (obj.compile (true, true, &ethers, &macs, &ips));

    ethers.push_back ("0x8888");
    assert (obj.compile (false, false, &ethers, nullptr, nullptr));
    assert (obj.compile (false, false, nullptr, &macs, nullptr));
    assert (obj.compile (false, false, nullptr, nullptr, &ips));
    assert (obj.compile (true, false, &ethers, nullptr, nullptr));
    assert (obj.compile (true, false, nullptr, &macs, nullptr));
    assert (obj.compile (true, false, nullptr, nullptr, &ips));
    assert (obj.compile (false, true, &ethers, nullptr, nullptr));
    assert (obj.compile (false, true, nullptr, &macs, nullptr));
    assert (obj.compile (false, true, nullptr, nullptr, &ips));
    assert (obj.compile (true, true, &ethers, nullptr, nullptr));
    assert (obj.compile (true, true, nullptr, &macs, nullptr));
    assert (obj.compile (true, true, nullptr, nullptr, &ips));
    assert (obj.compile (true, true, &ethers, &macs, nullptr));
    assert (obj.compile (true, true, &ethers, &macs, &ips));

    macs.push_back ("11:22:33:44:55:66");
    assert (obj.compile (false, false, &ethers, nullptr, nullptr));
    assert (obj.compile (false, false, nullptr, &macs, nullptr));
    assert (obj.compile (false, false, nullptr, nullptr, &ips));
    assert (obj.compile (true, false, &ethers, nullptr, nullptr));
    assert (obj.compile (true, false, nullptr, &macs, nullptr));
    assert (obj.compile (true, false, nullptr, nullptr, &ips));
    assert (obj.compile (false, true, &ethers, nullptr, nullptr));
    assert (obj.compile (false, true, nullptr, &macs, nullptr));
    assert (obj.compile (false, true, nullptr, nullptr, &ips));
    assert (obj.compile (true, true, &ethers, nullptr, nullptr));
    assert (obj.compile (true, true, nullptr, &macs, nullptr));
    assert (obj.compile (true, true, nullptr, nullptr, &ips));
    assert (obj.compile (true, true, &ethers, &macs, nullptr));
    assert (obj.compile (true, true, &ethers, &macs, &ips));

    ips.push_back ("5.6.7.8");
    assert (obj.compile (false, false, &ethers, nullptr, nullptr));
    assert (obj.compile (false, false, nullptr, &macs, nullptr));
    assert (obj.compile (false, false, nullptr, nullptr, &ips));
    assert (obj.compile (true, false, &ethers, nullptr, nullptr));
    assert (obj.compile (true, false, nullptr, &macs, nullptr));
    assert (obj.compile (true, false, nullptr, nullptr, &ips));
    assert (obj.compile (false, true, &ethers, nullptr, nullptr));
    assert (obj.compile (false, true, nullptr, &macs, nullptr));
    assert (obj.compile (false, true, nullptr, nullptr, &ips));
    assert (obj.compile (true, true, &ethers, nullptr, nullptr));
    assert (obj.compile (true, true, nullptr, &macs, nullptr));
    assert (obj.compile (true, true, nullptr, nullptr, &ips));
    assert (obj.compile (true, true, &ethers, &macs, nullptr));
    assert (obj.compile (true, true, &ethers, &macs, &ips));
}
#endif

