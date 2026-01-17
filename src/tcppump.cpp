// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
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


#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <new>          // std::bad_alloc

#include "tcppump.hpp"

#include "bug.hpp"
#include "settings.hpp"
#include "signal.hpp"
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
#include "random.hpp"


cTcpPump::cTcpPump(const char* name, const char* brief, const char* usage, const char* description,
    const char* version, const char* build, const char* buildDetails)
: cCmdlineApp (name, brief, usage, description, version, build, buildDetails)
{
    cRandom::create();

    memset (&options, 0, sizeof(options));
    options.repeat    = 1;
    options.timeRes   = "m";
    options.outFormat = "pcap";

    timeScale       = 0;
    realtimeMode    = false;
    ifc             = nullptr;

    addCmdLineOption (true, 'i', "interface", "IFC",
            "Specify the name of the network interface through which packets are sent."
#if HAVE_WINDOWS
            "It can either be the AdapterName (GUID) like \"{3F4A136A-2ED5-4226-9CB2-7A511E93CD48}\", "
            "or the so-called FriendlyName, which is changeable by the user. "
            "For example \"WiFi\" or \"Local Area Connection 1\"."
#endif
            , &options.ifc);
    addCmdLineOption (true, 0, "myip4", "IPV4",
            "Use the specified IPv4 address as the source IP address instead of the network interface's IP address.",
            &options.myIP);
    addCmdLineOption (true, 0, "myip6", "IPV6",
            "Use the specified IPv6 address as the source IP address instead of the network interface's IP address.",
            &options.myIPv6);
    addCmdLineOption (true, 0, "mymac", "MAC",
            "Use the specified MAC address as the source MAC address instead of the network interface's MAC address.",
            &options.myMAC);
    addCmdLineOption (true, 0, "mtu", "MTU",
            "Use the specified MTU instead of the network interface's MTU.", (int*)&options.mtu);
    addCmdLineOption (true, 0, "rand-smac",
            "Use a random source MAC address. This overwrites --mymac and any explicitly defined addresses in the packets.",
            &options.randSrcMac);
    addCmdLineOption (true, 0, "rand-dmac",
            "Use a random destination MAC address. This overwrites all explicitly defined destination MAC addresses in the packets.",
             &options.randDstMac);
    addCmdLineOption (true, 0, "overwrite-dmac", "MAC",
            "Overwrite the destination MAC address of all packets with the specified MAC address.", &options.overwriteDMAC);
    addCmdLineOption (true, 's', "script",
            "Read packets from script file instead of command-line.", &options.script);
    addCmdLineOption (true, "pcap", "SCALE",
            "Replay PCAP files of captured packets (e.g via Wireshark or tcpdump). "
            "The transmission time can be scaled using the optional SCALE parameter. "
            "The default value for SCALE is 1.0, meaning the file is played in real-time. "
            "A value of 2.0 slows playback to half speed, while 0.5 plays it at twice the speed. "
            "A value of 0 plays the file as quickly as possible."
            , &options.pcap, &options.pcapScaling);
    addCmdLineOption (true, 'l', "loop", "N",
            "Send all files/packets N times. Default: N = 1. If N = 0, packets will be sent infinitely "
            "until Ctrl+c is pressed.", &options.repeat);
    addCmdLineOption (true, 'd', "delay", "TIME", "Delay the packet transmission by TIME. "
            "Resolution depends on the -t parameter. By default, no delay is applied.",
            &options.delay);
    addCmdLineOption (true, 't', "resolution", "RESOLUTION",
            "Set the time resolution for packet transmission. This affects -d parameter as well as all timestamps in script files. "
            "Possible values are 'u'= microseconds, 'm'= milliseconds(default), 'c'= centiseconds and 's'= seconds",
            &options.timeRes);
    addCmdLineOption (true, 'w', nullptr, "OUTFILE",
            "Write raw packet data to OUTFILE, or to the standard output if OUTFILE is set to '-'.", &options.outfile);
    addCmdLineOption (true, 'F', nullptr, "FORMAT",
            "Set the file format of the output capture file written using the -w option. "
            "Supported formats are: 'pcap' (default), 'text', 'hexstream', 'hexdump'", &options.outFormat);
    addCmdLineOption (true, 'a', "arp",
            "Resolve the destination MAC address for IPv4 packets using ARP. "
            "If the destination MAC address is omitted in IPv4 packets, it will be automatically determined via ARP.",
            &options.arp);
    addCmdLineOption (true, 0, "predictable-random",
            "Use a simple sequence instead of random numbers to generate predictable values.", &options.testPredictableRandom);
}

cTcpPump::~cTcpPump()
{
    delete ifc;
    cRandom::destroy();
}

int cTcpPump::execute (const std::vector<std::string>& args)
{
    cMacAddress overwriteDMAC;
    double pcapScale = 1.0;

    // print packet syntax if requested and exit
    if (args.size() && !args[0].compare ("help"))
    {
        cInstructionParser::printProtocolList (args.size() > 1 ? args[1].c_str() : nullptr);
        return 0;
    }

    // ommiting of option -i is only allowed if output is written to a file (-w option) and
    // automatic mac address resolution is off
    if (!options.ifc && (!options.outfile || options.arp))
    {
        Console::PrintError ("Option -i --interface not set\n");
        return -1;
    }

    if (!args.size())
    {
        Console::PrintError (options.script ? "no script files provided\n": "no packet data provided\n");
        return -2;
    }

    if (options.testPredictableRandom)
        cRandom::setCounterMode(0);

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

    if (options.pcap && options.pcapScaling)
    {
        try
        {
            pcapScale = std::stod (options.pcapScaling);
        }
        catch (...)
        {
            Console::PrintError ("Unsupported pcap scaling value '%s'\n", options.pcapScaling);
            return -2;
        }
    }

    if (options.myIP)
    {
        if (!cSettings::get().setMyIPv4 (options.myIP))
        {
            Console::PrintError ("Wrong IPv4 address format %s\n", options.myIP);
            return -1;
        }
    }
    if (options.myIPv6)
    {
        if (!cSettings::get().setMyIPv6 (options.myIPv6))
        {
            Console::PrintError ("Wrong IPv6 address format %s\n", options.myIPv6);
            return -1;
        }
    }
    if (options.myMAC)
    {
        if (!cSettings::get().setMyMAC  (options.myMAC))
        {
            Console::PrintError ("Wrong MAC address format %s\n", options.myMAC);
            return -1;
        }
    }
    if (options.overwriteDMAC)
    {
        if (!overwriteDMAC.set (options.overwriteDMAC))
        {
            Console::PrintError ("Wrong MAC address format %s\n", options.overwriteDMAC);
            return -1;
        }
    }
    if (options.script && options.pcap)
    {
        Console::PrintError ("Options -s and -p can't be used at the same time.\n");
        return -1;
    }

    if (options.ifc)
    {
        ifc = cNetInterface::create (options.ifc, !options.outfile);
        if (!ifc->isReady())
            return -1;
    }

    if (!cSettings::get().isMacSet())
    {
        cMacAddress ifMAC;
        if (ifc && ifc->getMAC(ifMAC))
            cSettings::get().setMyMAC(ifMAC);
        else
            Console::PrintVerbose ("Warning: Could not determine mac address of interface or -i option not set.\n"
                                   "         Automatic source mac usage is will be disabled.\n");
    }
    if (!cSettings::get().isIPSet())
    {
        cIPv4 ifIPv4;
        if (ifc && ifc->getIPv4(ifIPv4))
            cSettings::get().setMyIPv4(ifIPv4);
        else
            Console::PrintVerbose ("Warning: Could not determine IPv4 address of interface or -i option not set.\n"
                                   "         Automatic source IPv4 usage is will be disabled.\n");
        }
    if (!cSettings::get().isIPv6Set())
    {
        cIPv6 ifIPv6;
        if (ifc && ifc->getIPv6(ifIPv6))
            cSettings::get().setMyIPv6(ifIPv6);
        else
            Console::PrintVerbose ("Warning: Could not determine IPv6 address of interface or -i option not set.\n"
                                   "         Automatic source IPv6 usage is will be disabled.\n");
        }
    if (options.mtu)
    {
        if (options.mtu < 68 || options.mtu > 1024*1024) // limit the configurable MTU
        {
            Console::PrintError ("MTU must be between 68 and 1048576\n");
            return -1;
        }
        cSettings::get().setMyMTU (options.mtu);
    }
    else
    {
        if (ifc)
        {
            unsigned mtu = (unsigned)ifc->getMTU();
            if (mtu)
                cSettings::get().setMyMTU (mtu);
            else
                Console::PrintError ("Warning: Could not determine MTU of interface or -i option not set. Using default value %u\n", cSettings::get().getMyMTU());
        }
    }


    activeDelay.setUs((uint64_t)options.delay * (uint64_t)timeScale);

    // Install a signal handler
    cSignal::sigintEnable ();

    cCompiler compiler (options.script ? cCompiler::SCRIPT : options.pcap ? cCompiler::PCAP : cCompiler::PACKET,
            activeDelay, timeScale, !!options.arp, pcapScale);
    cFilter    filter (options.overwriteDMAC ? &overwriteDMAC : nullptr);
    cScheduler scheduler;

    try
    {
        // Packet-flow-chain: args --> compiler -> filter -> resolver -> scheduler -> packetData
        // Each step may alter the content of packetData
        cPacketData& packetData = compiler  << args;

        if (!options.outfile && !ifc->open ())
            return -1;

        filter << packetData;
        if (ifc)
        {
            cResolver resolver (*ifc);
            resolver << packetData;
        }
        scheduler << packetData;

        // if user has set a default packet delay, real-time mode is ALWAYS enabled
        if (!activeDelay.isNull ())
            realtimeMode = true;
        else
            realtimeMode = packetData.hasUserTimestamps;

        // prepare backend for packet output
        cPreprocessor preprop(options.randSrcMac, options.randDstMac);
        cOutput backend (preprop);
        if (options.outfile)    // write output to file?
            backend.prepare (options.outfile, options.outFormat, options.repeat);
        else
            backend.prepare (*ifc, realtimeMode, options.repeat);

        Console::PrintMoreVerbose ("Will send %zu packets\n", packetData.getPacketCnt());
        if (options.repeat > 1)
            Console::PrintMoreVerbose ("Repeating %d times\n", options.repeat);
        else if (options.repeat == 0)
            Console::PrintMoreVerbose ("Repeating infinitely\n");
        if (realtimeMode)
            Console::PrintMoreVerbose ("Real-time mode with default delay between packets %" PRIu64 " usecs\n\n", activeDelay.us());
        else
            Console::PrintMoreVerbose ("Max. throughput mode\n\n");


        // send all the packets
        backend << packetData;

        uint64_t sentPackets, sentBytes; double duration;
        backend.statistic (sentPackets, sentBytes, duration);

        Console::PrintVerbose ("Successfully %s %" PRIu64 " %s. ", options.outfile ? "wrote" : "sent" ,sentPackets, sentPackets == 1 ? "packet" : "packets");
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
    catch(std::exception & e)
    {
        Console::PrintError ("std::exception (%s)\n", e.what ());
        BUG ("unexpected exception");
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
    BUG_ON (!e.errorMsg());
    BUG_ON (!e.instruction());
    std::string s, b;
    if (e.instruction() && e.errorBegin())
    {
        BUG_ON ((e.errorBegin() - e.instruction()) < 0);
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
            "tcppump -i IFC [OPTIONS] packets|infiles "
            "</br> tcppump -w OUTFILE [OPTIONS] packets|infiles",
            "'tcppump help' lists all available network protocol types. "
            "Use 'tcppump help <protocol type>' to show the detailed syntax of the specified protocol. "
            "</br> </br> Homepage: <https://github.com/amartin755/tcppump>",
            APP_VERSION, BUILD_TIME,  BUILD_TYPE "-" GIT_TAG "-" GIT_COMMIT);
    return app.main (argc, argv);
}
