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


#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <new>          // std::bad_alloc

#include "tcppump.hpp"

#include "bug.hpp"
#include "signal.hpp"
#include "sleep.hpp"
#include "interface.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "fileparser.hpp"
#include "fileioexception.hpp"
#include "compiler.hpp"
#include "resolver.hpp"
#include "filter.hpp"
#include "scheduler.hpp"
#include "preprocessor.hpp"
#include "output.hpp"


cTcpPump::cTcpPump(const char* name, const char* brief, const char* usage, const char* description)
: cCmdlineApp (name, brief, usage, description)
{
    memset (&options, 0, sizeof(options));
    options.repeat    = 1;
    options.timeRes   = "m";

    timeScale          = 0;
    realtimeMode       = false;

    addCmdLineOption (false, 'i', "interface", "IFC",
            "Name of the network interface via which the packets are sent."
#if HAVE_WINDOWS
            "\n\t"
            "It can either be the AdapterName (GUID) like \"{3F4A136A-2ED5-4226-9CB2-7A511E93CD48}\", \n\t"
            "or the so-called FriendlyName, which is changeable by the user.\n\t"
            "For example \"WiFi\" or \"Local Area Connection 1\"."
#endif
            , &options.ifc);
    addCmdLineOption (true, 0, "myip4", "IPV4",
            "Use IPV4 as source IPv4 address instead of the network adapters ip address", &options.myIP);
    addCmdLineOption (true, 0, "mymac", "MAC",
            "Use MAC as source MAC address instead of the network adapters MAC address", &options.myMAC);
    addCmdLineOption (true, 0, "rand-smac",
            "Use random source MAC address. Overwrites --mymac as well as explicitly defined addresses in packets.", &options.randSrcMac);
    addCmdLineOption (true, 0, "rand-dmac",
            "Use random destination MAC address. Overwrites all explicitly defined addresses in packets.", &options.randDstMac);
    addCmdLineOption (true, 'v', "verbose",
            "When parsing and printing, produce verbose output. This option can be supplied multiple times\n\t"
            "(max. 4 times, i.e. -vvvv) for even more debug output. "
            , &options.verbosity);
    addCmdLineOption (true, 's', "script", "Packets are defined in script files, that contain token based packets.", &options.script);
#if HAVE_PCAP
    addCmdLineOption (true, 'p', "pcap", "pcap file of captured packets (e.g via wireshark or tcpdump) will be replayed.", &options.pcap);
#endif
    addCmdLineOption (true, 'l', "loop", "N",
            "Send all files/packets N times. Default: N = 1. If N = 0, packets will be sent infinitely\n\t"
            "until ctrl+c is pressed.", &options.repeat);
    addCmdLineOption (true, 'd', "delay", "TIME", "Packet transmission is delayed TIME."
            "Resolution depends on -t parameter. Default is no delay.", &options.delay);
    addCmdLineOption (true, 't', "resolution", "RESOLUTION",
            "Resolution of transmission time. This affects -d parameter as well as all timestamps in script files.\n\t"
            "Possible values are 'u'= microseconds, 'm'= milliseconds(default), 'c'= centiseconds and 's'= seconds" , &options.timeRes);
#if HAVE_PCAP
    addCmdLineOption (true, 'o', "write-to-file", "OUTFILE", "Write generated packets to pcap file OUTFILE instead of sending them to the network.", &options.outpcap);
#endif
    addCmdLineOption (true, 'a', "arp",
            "Resolve destination MAC address for IPv4 packets.\n\t"
            "If dmac parameter of IPv4 based packets is ommited, the destination MAC will be"
            "automatically detremined via ARP.",
            &options.arp);
}

cTcpPump::~cTcpPump()
{
    // TODO Auto-generated destructor stub
}

int cTcpPump::execute (const std::list<std::string>& args)
{
    cMacAddress ownMac;
    cIpAddress  ownIP;

    std::srand ((unsigned)std::time (NULL));


    switch (options.verbosity)
    {
    case 1:
        Console::SetPrintLevel(Console::Verbose);
        break;
    case 2:
        Console::SetPrintLevel(Console::MoreVerbose);
        break;
    case 3:
        Console::SetPrintLevel(Console::MostVerbose);
        break;
    case 4:
        Console::SetPrintLevel(Console::Debug);
        break;
    }

    switch (options.timeRes[0])
    {
    case 'u':    // microseconds
        timeScale = 1;
        break;
    case 'm':   // milliseconds
        timeScale = 1000;
        break;
    case 'c':   // centiseconds
        timeScale = 10000;
        break;
    case 's':  // seconds
        timeScale = 1000000;
        break;
    default:
        Console::PrintError ("Unsupported time resolution '%c'\n", options.timeRes[0]);
        return -2;
    }

    if (!args.size())
    {
        Console::PrintError (options.script ? "no script files provided\n": "no packet data provided\n");
        return -2;
    }

    if (options.myIP)
    {
        if (!ownIP.set (options.myIP))
        {
            Console::PrintError ("Wrong IPv4 address format %s\n", options.myIP);
            return -1;
        }
    }
    if (options.myMAC)
    {
        if (!ownMac.set (options.myMAC))
        {
            Console::PrintError ("Wrong MAC address format %s\n", options.myMAC);
            return -1;
        }
    }

    cInterface ifc (options.ifc);
    if (!ifc.open())
        return -1;
    if (!options.myMAC && !ifc.getMAC(ownMac))
    {
        Console::PrintError ("Could not determine mac address of interface.\n");
        return -1;
    }
    if (!options.myIP && !ifc.getIPv4(ownIP))
    {
        Console::PrintError ("Could not determine IPv4 address of interface.\n");
        return -1;
    }

    if (options.script && options.pcap)
    {
        Console::PrintError ("Options -s and -p can't be used at the same time.\n");
        return -1;
    }

    activeDelay.setUs(options.delay * timeScale);

    cTimeval accuracy = tcppump::SleepInit ();
    Console::PrintMostVerbose ("System timer accuracy is %u usec. For packet delays below that value we do busy waiting.\n", (unsigned)accuracy.us());

    // Install a signal handler
    cSignal::sigintEnable ();

    cCompiler compiler (options.script ? cCompiler::SCRIPT : options.pcap ? cCompiler::PCAP : cCompiler::PACKET,
            ownMac, ownIP, activeDelay, timeScale, !!options.arp);
    cFilter    filter;
    cResolver  resolver (ifc);
    cScheduler scheduler;

    try
    {
        // Packet-flow-chain: args --> compiler -> filter -> resolver -> scheduler -> packetData
        // Each step may alter the content of packetData
        cPacketData& packetData = compiler  << args;
                                  filter    << packetData;
                                  resolver  << packetData;
                                  scheduler << packetData;



        // if user has set a default packet delay, real-time mode is ALWAYS enabled
        if (!activeDelay.isNull ())
            realtimeMode = true;
        else
            realtimeMode = packetData.hasUserTimestamps;

        // prepare backend for packet output
        cOutput backend (cPreprocessor (options.randSrcMac, options.randDstMac));
#if HAVE_PCAP
        if (options.outpcap)    // write output to pcap file?
            backend.prepare (options.outpcap, options.repeat);
        else
#endif
            backend.prepare (ifc, realtimeMode, options.repeat);

        Console::PrintMoreVerbose ("Sending %d packets\n", packetData.packets.size());
        if (options.repeat)
            Console::PrintMoreVerbose ("Repeating %d times\n", options.repeat);
        else
            Console::PrintMoreVerbose ("Repeating infinitely\n");
        if (realtimeMode)
            Console::PrintMoreVerbose ("Real-time mode with default delay between packets %" PRIu64 " usecs\n\n", activeDelay);
        else
            Console::PrintMoreVerbose ("Max. throughput mode\n\n");

        // send all the packets
        backend << packetData;

        uint64_t sentPackets, sentBytes; double duration;
        backend.statistic (sentPackets, sentBytes, duration);

        Console::PrintVerbose ("Successfully %s %" PRIu64 " %s. ", options.outpcap ? "wrote" : "sent" ,sentPackets, sentPackets == 1 ? "packet" : "packets");
        if (duration > 0.0)
            Console::PrintVerbose ("%" PRIu64 " bytes in %f seconds (= %f Mbit/s)", sentBytes, duration, ((sentBytes*8)/duration)/1000000.0);
        Console::PrintVerbose ("\n");

        return !sentPackets;
    }
    catch (FileParseException &e)
    {
        printFileParseError (e);
        return -2;
    }
    catch (ParseException &e)
    {
        printParseError (e);
        return -2;
    }
    catch (FileIOException &e)
    {
        Console::PrintError ("%s %s\n", e.what(), e.value());
        return -2;
    }
    catch (std::runtime_error &e)
    {
        Console::PrintError ("%s\n", e.what());
        return -2;
    }
    catch (std::bad_alloc& e)
    {
        Console::PrintError ("Not enough memory (%s)\n", e.what ());
        return -2;
    }
    catch (...)
    {
        BUG ("unexpected exception");
    }

    BUG ("unreachable code");
    return 0;
}


void cTcpPump::printParseError (const ParseException &e) const
{
    BUG_ON (e.errorMsg());
    BUG_ON (e.instruction());
    std::string s, b;
    if (e.instruction() && e.errorBegin())
    {
        BUG_ON ((e.errorBegin() - e.instruction()) >= 0);
        s.assign(e.errorBegin() - e.instruction(), ' ');
    }
    if (e.errorLen())
        b.assign(e.errorLen()-1, '~');
    if (e.details())
        Console::PrintError ("error: %s '%s'\n  %s\n  %s^%s\n", e.errorMsg (), e.details (), e.instruction (), s.c_str(), b.c_str());
    else
        Console::PrintError ("error: %s\n  %s\n  %s^%s\n", e.errorMsg (), e.instruction (), s.c_str(), b.c_str());
}


void cTcpPump::printFileParseError (const FileParseException &e) const
{
    Console::PrintError ("%s (line %d) ", e.filePath(), e.lineNumber());

    if (e.instruction())
        printParseError (e);
    else
        Console::PrintError ("error: %s\n", e.errorMsg ());
}


int main(int argc, char* argv[])
{
    cTcpPump app (
            "tcppump",
            "An Ethernet packet generator",
            "tcppump -i IFC [OPTIONS] packets/infiles",
            "Homepage: <https://github.com/amartin755/tcppump>");
    return app.main (argc, argv);
}
