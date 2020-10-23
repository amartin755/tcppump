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

#include "tcppump.hpp"

#include "bug.h"
#include "signal.hpp"
#include "sleep.hpp"
#include "getch.hpp"
#include "interface.hpp"
#include "dissector.hpp"
#include "ipaddress.hpp"
#include "macaddress.hpp"
#include "instructionparser.hpp"
#include "fileparser.hpp"
#include "ethernetpacket.hpp"
#if HAVE_PCAP
#include "pcapfileio.hpp"
#endif
#include "parsehelper.hpp"


cTcpPump::cTcpPump(const char* name, const char* brief, const char* usage, const char* description)
: cCmdlineApp (name, brief, usage, description)
{
    memset (&options, 0, sizeof(options));
    options.repeat    = 1;
    options.inputmode = "token";
    options.keys      = "1234567890";
    options.timeRes   = "m";

    triedToSendPackets = 0;
    sentPackets        = 0;
    malformedPackets   = 0;
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
    addCmdLineOption (true, 0, "input", "TYPE",
            "Input format of the packets to be sent. Possible values for TYPE (default is \"token\") are:\n\t"
            "raw     Packets are defined as hex-ascii string, and will not be interpreted.\n\t"
            "        example: 0123456789ABCDEF001122334455667788\n\t"
            "token   Token based definition of packets. tcppump will compile it to Ethernet packets.\n\t"
            "        example: eth: .dest=44:22:33:44:55:66 .payload=1234567890abcdef\n\t"
            "        For complete description of the syntax, see documentation.\n\t"
            "script  Packets are defined in script files, that contain token based packets."
#if HAVE_PCAP
            "\n\t"
            "pcap    pcap file of captured packets (e.g via wireshark or tcpdump) will be replayed."
#endif
            , &options.inputmode);
    addCmdLineOption (true, 'r', "raw", "Short for --input=raw", &options.raw);
    addCmdLineOption (true, 's', "script", "Short for --input=script", &options.script);
#if HAVE_PCAP
    addCmdLineOption (true, 'p', "pcap", "Short for --input=pcap", &options.pcap);
#endif
    addCmdLineOption (true, 'l', "loop", "N",
            "Send all files/packets N times. Default: N = 1. If N = 0, packets will be sent infinitely\n\t"
            "until ctrl+c is pressed.", &options.repeat);
    addCmdLineOption (true, 'd', "delay", "TIME", "Packet transmission is delayed TIME."
            "Resolution depends on -t parameter. Default is no delay.", &options.delay);
    addCmdLineOption (true, 't', "resolution", "RESOLUTION",
            "Resolution of transmission time. This affects -d parameter as well as all timestamps in script files.\n\t"
            "Possible values are 'u'= microseconds, 'm'= milliseconds(default), 'c'= centiseconds and 's'= seconds" , &options.timeRes);
    addCmdLineOption (true, "interactive", "KEYLIST",
            "Enable interactive mode (EXPERIMENTAL). In interactive mode no packets are sent automatically.\n\t"
            "Instead the packets are bound to keys and only sent when the corresponding key\n\t"
            "is pressed. The default implementation binds the first 10 packets to the keys 1, 2, ... 0."
            , &options.interactive, &options.keys);
#if HAVE_PCAP
    addCmdLineOption (true, 'o', "write-to-file", "OUTFILE", "Write generated packets to pcap file OUTFILE instead of sending them to the network.", &options.outpcap);
#endif
    addCmdLineOption (true, 0, "dissect",
            "Prints the dissected content of sent packets as known from tcpdump.", &options.dissect);
}

cTcpPump::~cTcpPump()
{
    // TODO Auto-generated destructor stub
}

int cTcpPump::execute (int argc, char* argv[])
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

    if (!argc)
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

    if (strcmp ("token", options.inputmode))
    {
        if (!strcmp ("raw", options.inputmode))
            options.raw = true;
        else if (!strcmp ("script", options.inputmode))
            options.script = true;
        else if (!strcmp ("pcap", options.inputmode))
            options.pcap = true;
        else
        {
            Console::PrintError ("Unknown --input=%s.\n", options.inputmode);
            return -1;
        }
    }

    activeDelay.setUs(options.delay * timeScale);

    cTimeval accuracy = tcppump::SleepInit ();
    Console::PrintMostVerbose ("System timer accuracy is %u usec. For packet delays below that value we do busy waiting.\n", (unsigned)accuracy.us());

    bool ok = options.script ? parseScripts (ownMac, ownIP, argc, argv) :
#if HAVE_PCAP
              options.pcap   ? parsePcapFiles (argc, argv)              :
#endif
                               parsePackets (ownMac, ownIP, argc, argv);
    if (!ok)
        return -2;

#if HAVE_PCAP
    if (options.outpcap)    // write output to pcap file?
    {
        if (!outfile.open (options.outpcap, true))
            return -3;
    }
#endif

    // Install a signal handler
    cSignal::sigintEnable ();

    // if user has set a default packet delay, real-time mode is ALWAYS enabled
    if (!activeDelay.isNull ())
        realtimeMode = true;

    uint64_t sentBytes = 0;
    double seconds = 0;

    if (!options.interactive)
    {
        cTimeval currentTime;
        BUG_ON (packets.size() == delays.size());

        Console::PrintMoreVerbose ("Sending %d packets\n", packets.size());
        bool endless = !options.repeat;
        if (!endless)
            Console::PrintMoreVerbose ("Repeating %d times\n", options.repeat);
        else
            Console::PrintMoreVerbose ("Repeating infinitely\n", options.repeat);
        if (realtimeMode)
            Console::PrintMoreVerbose ("Real-time mode with default delay between packets %" PRIu64 " usecs\n\n", activeDelay);
        else
            Console::PrintMoreVerbose ("Max. throughput mode\n\n");


        ifc.prepareSendQueue(packets.size() * options.repeat, packets.size() * options.repeat * cEthernetPacket::MAX_DOUBLE_TAGGED_PACKET, realtimeMode);

        auto t1 = std::chrono::high_resolution_clock::now();
        while (!cSignal::sigintSignalled() && (endless || options.repeat--))
        {
            std::list<cTimeval>::const_iterator t = delays.cbegin();

            for (auto & p : packets)
            {
                currentTime.add (*t);
                if (options.randSrcMac)
                    p.setRandomSrcMac();
                if (options.randDstMac)
                    p.setRandomDestMac();
                if (!sendPacket (ifc, currentTime, p))
                    return -4;
                t++;
                sentBytes += p.getLength ();
            }
        }
        ifc.flushSendQueue();
        auto t2 = std::chrono::high_resolution_clock::now();
        auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        seconds = (double)elapsedUs.count()/1000000.0;
    }
    else
    {
        interactiveMode(ifc);
    }

    Console::PrintVerbose ("Successfully sent %u of %u packets. ", sentPackets, triedToSendPackets);
    if (seconds > 0.0)
        Console::PrintVerbose ("%" PRIu64 " bytes in %f seconds (= %f Mbit/s)", sentBytes, seconds, ((sentBytes*8)/seconds)/1000000.0);
    Console::PrintVerbose ("\n");

    return !sentPackets;
}


bool cTcpPump::parsePackets (const cMacAddress& ownMac, const cIpAddress& ownIP, int argc, char* argv[])
{
    cInstructionParser::cResult result (packets);
    cTimeval timestamp, currtime;

    Console::PrintDebug ("Parsing %d packets (format='%s') ...\n", argc, options.raw ? "raw" : "tokens");

    for (int n = 0; n < argc; n++)
    {
        try
        {
            if (!options.raw)
            {
                result.timestamp = activeDelay.us()/timeScale;
                result.isAbsolute = false;
                int count = cInstructionParser (ownMac, ownIP).parse (argv[n], result);
                timestamp.setUs(result.timestamp * timeScale);
                realtimeMode = realtimeMode || result.timeValid;

                for (int n = 0; n < count; n++)
                {
                    if (!result.isAbsolute)
                    {
                        delays.push_back (timestamp);
                        currtime.add (timestamp);
                    }
                    else
                    {
                        if (timestamp < currtime)  // FIXME What to do if timestamp < currtime? delay = 0 or parse exception?
                            BUG_ON ("fixme" == 0);
                        else
                        {
                            cTimeval delta(timestamp);
                            delays.push_back (timestamp.sub (currtime));
                            currtime.set (timestamp);
                        }
                    }
                }
            }
            else
            {
                cEthernetPacket packet;
                size_t len;
                const uint8_t* data = cParseHelper::hexStringToBin(argv[n], 0, len);
                if (!data)
                    throw FormatException (exParFormat, argv[n]);

                packet.setRaw (data, len);
                packets.push_back (std::move(packet));
                timestamp.setUs (activeDelay.us());
                delays.push_back(timestamp);
                delete[] data;
            }
        }
        catch (ParseException &e)
        {
            printParseError (e);
            return false;
        }
        catch (FormatException& e)
        {
            Console::PrintError ("%s %s\n", e.why (), e.value ());
            return false;
        }
    }
    return true;
}


bool cTcpPump::parseScripts (const cMacAddress& ownMac, const cIpAddress& ownIP, int scriptsCnt, char* scripts[])
{
    cInstructionParser::cResult result (packets);
    FILE *fp;
    int count;
    cFileParser parser;
    cTimeval timestamp, currtime, scriptStartTime;

    Console::PrintDebug ("Parsing %d script files ...\n", scriptsCnt);

    do
    {
        Console::PrintDebug ("Open '%s'\n", *scripts);

        if ((fp = fopen (*scripts, "rt")) == NULL)
        {
            Console::PrintError ("Unable to open the script file %s.\n", *scripts);
            return false;
        }

        parser.init (fp, activeDelay.us()/timeScale, ownMac, ownIP);

        scriptStartTime = currtime;

        do
        {
            // allocate a new packet
            try
            {
                count = parser.parse (result);
                timestamp.setUs(result.timestamp * timeScale);
                realtimeMode = realtimeMode || result.timeValid;

                for (int n = 0; n < count; n++)
                {
                    if (!result.isAbsolute)
                    {
                        delays.push_back (timestamp);
                        currtime.add (timestamp);
                    }
                    else
                    {
                        timestamp.add(scriptStartTime);

                        if (timestamp < currtime) // FIXME What to do if timestamp < currtime? delay = 0 or parse exception?
                            BUG ("FIXME");
                        else
                        {
                            cTimeval delta(timestamp);
                            delays.push_back (delta.sub (currtime));
                            currtime.set (timestamp);
                        }
                    }
                }
            }
            catch (FileParseException &e)
            {
                printParseError (*scripts, e);
                count = PARSE_ERROR;
            }
            catch (...)
            {
                Console::PrintError ("Not enough memory\n");
                return false;
            }

        }while (count > 0);

        fclose (fp);

        if (count != EOF)
            return false;

        scripts++;

    }while (--scriptsCnt > 0);

    return true;
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


void cTcpPump::printParseError (const char* filename, const FileParseException &e) const
{
    Console::PrintError ("%s (line %d) ", filename, e.lineNumber());
    printParseError (e);
}



#if HAVE_PCAP
bool cTcpPump::parsePcapFiles (int pcapCnt, char* pcaps[])
{
    Console::PrintDebug ("Parsing %d PCAP files ...\n", pcapCnt);

    do
    {
        cPcapFileIO pcap;
        cTimeval t;
        int len;

        Console::PrintDebug ("Open '%s'\n", *pcaps);

        if (pcapCnt && !pcap.open (*pcaps, false))
        {
            Console::PrintError ("Unable to open the pcap file %s.\n", *pcaps);
            return false;
        }

        uint8_t* data;

        while ((data = pcap.read(&t, &len)) != nullptr)
        {
            try
            {
                cEthernetPacket packet;
                packet.setRaw (data, len);

                packets.push_back (std::move(packet));
            }
            catch (...)
            {
                Console::PrintError ("Not enough memory\n");
                return false;
            }
        }

        pcap.close ();

    }while (--pcapCnt > 0);

    return true;
}
#endif


bool cTcpPump::sendPacket (cInterface &ifc, const cTimeval &t, const cEthernetPacket &p)
{
    triedToSendPackets++;

    if (!options.outpcap)
    {
        if(!ifc.sendPacket (p.get(), p.getLength(), t))
        {
            Console::PrintError ("Could not send packet.\n");
            return false;
        }
    }
#if HAVE_PCAP
    else
    {
        if (!outfile.write (t, p.get(), (int)p.getLength(), true))
            return false;
    }
#endif

    sentPackets++;
    if (options.dissect && !cDissector(p).dissect())
        malformedPackets++;

    return true;
}


bool cTcpPump::interactiveMode (cInterface &ifc)
{
    //TODO add some useful output to guide the user

    int n = 0;
    for (cEthernetPacket& p : packets)
    {
        if (options.keys[n] == '\0')
            break;

        keyBindings.insert (std::pair<int, cEthernetPacket&>((int)options.keys[n++], p));
    }

    int key;

    while ((key = tcppump::getch ()) != EOF)
    {
        if (cSignal::sigintSignalled())
            break;

        try
        {
            cEthernetPacket& p = keyBindings.at (key);
            if (!sendPacket (ifc, activeDelay, p))
                return false;
        }
        catch (const std::out_of_range&)
        {
            // key not found --> nothing to do
        }
    }

    return true;
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
