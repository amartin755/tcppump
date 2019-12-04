/**
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2019 Andreas Martin (netnag@mailbox.org)
 *
 * tcppump.cpp
 *
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

#include "tcppump.hpp"
#include "sleep.hpp"
#include "getch.hpp"
#include "interface.hpp"
#include "dissector.hpp"
#include "libnetnag/converter.hpp"
#include "libnetnag/instructionparser.hpp"
#include "libnetnag/fileparser.hpp"
#include "libnetnag/ethernetpacket.hpp"
#if HAVE_PCAP
#include "libnetnag/pcapfileio.hpp"
#endif

using namespace nn;

cTcpPump::cTcpPump(const char* name, const char* brief, const char* usage, const char* description)
: cCmdlineApp (name, brief, usage, description)
{
    memset (&options, 0, sizeof(options));
    options.repeat    = 1;
    options.delay     = 0;
    options.inputmode = "token";
    options.keys      = "1234567890";

    triedToSendPackets = 0;
    sentPackets        = 0;
    malformedPackets   = 0;

    addCmdLineOption (false, 'i', "interface", "IFC",
            "Name of the network interface via which the packets are sent. On Linux this can be one of\n\t"
            "the interfaces that are printed by \"ip link\" or \"ifconfig\", for example \"eth0\".\n\t"
            "On Windows it can either be the AdapterName (GUID) like \"{3F4A136A-2ED5-4226-9CB2-7A511E93CD48}\", \n\t"
            "or the so-called FriendlyName, which is changeable by the user.\n\t"
            "For example \"WiFi\" or \"Local Area Connection 1\"."
            , &options.ifc);
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
    addCmdLineOption (true, 'p', "pcap", "Short for --input=pcap", &options.pcap);
    addCmdLineOption (true, 'l', "loop", "N", "Send all files/packets N times. Default: N = 1", &options.repeat);
    addCmdLineOption (true, 'd', "delay", "SECONDS", "Packet transmission is delayed SECONDS. Default is no delay", &options.delay);
    addCmdLineOption (true, "interactive", "KEYLIST",
            "Enable interactive mode (EXPERIMENTAL). In interactive mode no packets are sent automatically.\n\t"
            "Instead the packets are bound to keys and only sent when the corresponding key\n\t"
            "is pressed. The default implementation binds the first 10 packets to the keys 1, 2, ... 0."
            , &options.interactive, &options.keys);
}

cTcpPump::~cTcpPump()
{
    // TODO Auto-generated destructor stub
}

int cTcpPump::execute (int argc, char* argv[])
{
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

    if (!argc)
    {
        Console::PrintError (options.script ? "no script files provided\n": "no packet data provided\n");
        return -2;
    }

    cInterface ifc (options.ifc);
    if (!ifc.open())
        return -1;
    mac_t ownMac;
    if (!ifc.getMAC(&ownMac))
    {
        Console::PrintError ("Could not determine mac address of interface.\n");
        return -1;
    }
    ipv4_t ownIP;
    if (!ifc.getIPv4(&ownIP))
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

    bool ok = options.script ? parseScripts (ownMac, ownIP, argc, argv) :
    		  options.pcap   ? parsePcapFiles (argc, argv)              :
    				           parsePackets (ownMac, ownIP, argc, argv);
    if (!ok)
        return -2;

    if (!options.interactive)
    {
        Console::PrintMoreVerbose ("Sending %d packets, each delayed by %d seconds. Repeating %d times.\n\n", packets.size(), options.delay, options.repeat);
        while (options.repeat--)
        {
            for (cEthernetPacket& p : packets)
            {
                if (!sendPacket (ifc, options.delay, p))
                    return -4;
            }
        }
    }
    else
    {
        interactiveMode(ifc);
    }

    Console::PrintVerbose ("Successfully sent %u of %u packets. %u of them where malformed\n", sentPackets, triedToSendPackets, malformedPackets);
    return 0;
}


bool cTcpPump::parsePackets (mac_t ownMac, ipv4_t ownIP, int argc, char* argv[])
{
    cTimeval timestamp;
    bool isAbsolute;

    for (int n = 0; n < argc; n++)
    {
    	Console::PrintDebug ("Parsing %d packets (format='%s') ...\n", argc, options.raw ? "raw" : "tokens");
        try
        {
            cEthernetPacket packet;
            if (!options.raw)
                cInstructionParser (ownMac, ownIP).parse (argv[n], timestamp, isAbsolute, packet);
            else
                packet.setRaw (argv[n], strlen (argv[n]));

            packets.push_back (std::move(packet));
        }
        catch (ParseException &e)
        {
            Console::PrintError ("%s %s\n", e.what (), e.value ());
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


bool cTcpPump::parseScripts (mac_t ownMac, ipv4_t ownIP, int scriptsCnt, char* scripts[])
{
    FILE *fp;
    int len;
    cFileParser parser;
    cTimeval timestamp;
    bool isAbsolute;

	Console::PrintDebug ("Parsing %d script files ...\n", scriptsCnt);

    do
    {
    	Console::PrintDebug ("Open '%s'\n", *scripts);

    	if (scriptsCnt && ((fp = fopen (*scripts, "rt")) == NULL))
        {
            Console::PrintError ("Unable to open the script file %s.\n", *scripts);
            return false;
        }

        parser.init (fp, ownIP, ownMac, 0);

        do
        {
            // allocate a new packet
            try
            {
                cEthernetPacket packet;
                len = parser.parse (timestamp, isAbsolute, packet);
                if (len > 0)
                    packets.push_back (std::move(packet));
            }
            catch (...)
            {
                Console::PrintError ("Not enough memory\n");
                return false;
            }

        }while (len > 0);

        fclose (fp);

        if (len == PARSE_ERROR)
            Console::PrintError ("%s %s\n", *scripts, parser.getLastError ());
        if (len != EOF)
            return false;

        scripts++;

    }while (--scriptsCnt > 0);

    return true;
}


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


bool cTcpPump::sendPacket (cInterface &ifc, unsigned delay, cEthernetPacket &p)
{
    if (delay)
    {
        Console::PrintVerbose ("Waiting %d seconds\n", options.delay);
        tcppump::Sleep (options.delay);
    }
    triedToSendPackets++;
    if(!ifc.sendPacket (p.get(), p.getLength()))
    {
        Console::PrintError ("Could not send packet.\n");
        return false;
    }
    sentPackets++;
    if (!cDissector(p).dissect())
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
        try
        {
            cEthernetPacket& p = keyBindings.at (key);
            if (!sendPacket (ifc, options.delay, p))
                return false;
        }
        catch (const std::out_of_range& e)
        {
            // key not found --> nothing to do
        }
    }

    return true;
}


int main(int argc, char* argv[])
{
    cTcpPump app ("tcppump", "An Ethernet packet generator", "tcppump -i IFC [OPTIONS] packets/infiles", "TODO description");
    return app.main (argc, argv);
}
